#include "../src/parsers/Encoding.hpp"
#include <iostream>
#include <cassert>
#include <string>
#include <string_view>
#include <initializer_list>

using namespace finance::parsers;

namespace {

// Вспомогательная функция для создания байтовой строки CP1251 из шестнадцатеричных кодов
std::string makeBytes(std::initializer_list<uint8_t> bytes) {
    std::string s;
    s.reserve(bytes.size());
    for (uint8_t b : bytes) {
        s.push_back(static_cast<char>(b));
    }
    return s;
}

void test_ascii() {
    std::string_view ascii_text = "Hello, World! 12345; ./<>?~!@#$%^&*()_+-=";
    assert(cp1251ToUtf8(ascii_text) == ascii_text);
    std::cout << "[PASS] test_ascii" << std::endl;
}

void test_cp1251_individual_symbols() {
    // Белорусские буквы и спецсимволы по ТЗ
    assert(cp1251ToUtf8(makeBytes({0xA1})) == "Ў");
    assert(cp1251ToUtf8(makeBytes({0xA2})) == "ў");
    assert(cp1251ToUtf8(makeBytes({0xB2})) == "І");
    assert(cp1251ToUtf8(makeBytes({0xB3})) == "і");
    assert(cp1251ToUtf8(makeBytes({0xA8})) == "Ё");
    assert(cp1251ToUtf8(makeBytes({0xB8})) == "ё");
    assert(cp1251ToUtf8(makeBytes({0xB9})) == "№");
    assert(cp1251ToUtf8(makeBytes({0xAB})) == "«");
    assert(cp1251ToUtf8(makeBytes({0xBB})) == "»");

    // 3-байтовые спецсимволы (евро €, многоточие …)
    assert(cp1251ToUtf8(makeBytes({0x88})) == "€");
    assert(cp1251ToUtf8(makeBytes({0x85})) == "…");

    std::cout << "[PASS] test_cp1251_individual_symbols" << std::endl;
}

void test_cp1251_bank_statement_phrase() {
    // Реальная фраза:
    // «Прыорбанк» ААТ, аплата №12345: Ў, ў, І, і, Ё, ё
    std::string cp1251_phrase = makeBytes({
        0xAB,                                     // «
        0xCF, 0xF0, 0xFB, 0xEE, 0xF0, 0xE1, 0xE0, 0xED, 0xEA, // Прыорбанк
        0xBB,                                     // »
        ' ',
        0xC0, 0xC0, 0xD2,                         // ААТ
        ',', ' ',
        0xE0, 0xEF, 0xEB, 0xE0, 0xF2, 0xE0,       // аплата
        ' ',
        0xB9,                                     // №
        '1', '2', '3', '4', '5', ':', ' ',
        0xA1, ',', ' ',                           // Ў,
        0xA2, ',', ' ',                           // ў,
        0xB2, ',', ' ',                           // І,
        0xB3, ',', ' ',                           // і,
        0xA8, ',', ' ',                           // Ё,
        0xB8                                      // ё
    });

    std::string expected_utf8 = "«Прыорбанк» ААТ, аплата №12345: Ў, ў, І, і, Ё, ё";
    std::string converted = cp1251ToUtf8(cp1251_phrase);

    assert(converted == expected_utf8);
    std::cout << "[PASS] test_cp1251_bank_statement_phrase" << std::endl;
}

void test_is_valid_utf8() {
    // Валидные UTF-8 строки
    assert(isValidUtf8(""));
    assert(isValidUtf8("Plain ASCII text 123"));
    assert(isValidUtf8("«Прыорбанк» ААТ, аплата №12345: Ў, ў, І, і, Ё, ё"));
    assert(isValidUtf8("Сумма: 100€")); // 3 байта
    assert(isValidUtf8("Emoji test: 🚀💰")); // 4 байта

    // Невалидные UTF-8 последовательности
    // 1. Одинокий байт-продолжение без стартового
    assert(!isValidUtf8(makeBytes({0x80})));
    assert(!isValidUtf8(makeBytes({0xBF})));
    assert(!isValidUtf8(makeBytes({'A', 0x80, 'B'})));

    // 2. Оборванный 2-байтовый символ (стартовый байт 0xD0 без продолжения)
    assert(!isValidUtf8(makeBytes({0xD0})));

    // 3. Оборванный 3-байтовый символ
    assert(!isValidUtf8(makeBytes({0xE0, 0xA0})));

    // 4. Оборванный 4-байтовый символ
    assert(!isValidUtf8(makeBytes({0xF0, 0x90, 0x80})));

    // 5. Недопустимые байты (0xFF, 0xFE)
    assert(!isValidUtf8(makeBytes({0xFF})));
    assert(!isValidUtf8(makeBytes({0xFE})));

    // 6. Overlong sequence (например 0xC0 или 0xC1)
    assert(!isValidUtf8(makeBytes({0xC0, 0xAF})));
    assert(!isValidUtf8(makeBytes({0xC1, 0x80})));

    // 7. Сырые байты CP1251 кириллицы (невалидны как UTF-8)
    std::string cp1251_text = makeBytes({0xCF, 0xF0, 0xE8, 0xE2, 0xE5, 0xF2}); // "Привет" в CP1251
    assert(!isValidUtf8(cp1251_text));

    std::cout << "[PASS] test_is_valid_utf8" << std::endl;
}

void test_ensure_utf8() {
    std::string valid_utf8 = "«Прыорбанк» аплата №9876";
    std::string cp1251_bytes = makeBytes({
        0xAB,                                     // «
        0xCF, 0xF0, 0xFB, 0xEE, 0xF0, 0xE1, 0xE0, 0xED, 0xEA, // Прыорбанк
        0xBB,                                     // »
        ' ',
        0xE0, 0xEF, 0xEB, 0xE0, 0xF2, 0xE0,       // аплата
        ' ',
        0xB9,                                     // №
        '9', '8', '7', '6'
    });

    // 1. Для CP1251 строки — перекодирует в UTF-8
    assert(ensureUtf8(cp1251_bytes) == valid_utf8);

    // 2. Для уже валидной UTF-8 строки — не ломает («защита от кракозябр»)
    assert(ensureUtf8(valid_utf8) == valid_utf8);

    // 3. Идемпотентность: двойной вызов не портит результат
    assert(ensureUtf8(ensureUtf8(cp1251_bytes)) == valid_utf8);

    std::cout << "[PASS] test_ensure_utf8" << std::endl;
}

} // namespace

int main() {
    std::cout << "Running Encoding tests..." << std::endl;

    test_ascii();
    test_cp1251_individual_symbols();
    test_cp1251_bank_statement_phrase();
    test_is_valid_utf8();
    test_ensure_utf8();

    std::cout << "All Encoding tests successfully passed!" << std::endl;
    return 0;
}

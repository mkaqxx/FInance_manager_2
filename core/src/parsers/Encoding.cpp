#include "Encoding.hpp"
#include <array>
#include <cstdint>

namespace finance::parsers {
    namespace {

        // Таблица соответствия байтов 0x80..0xFF кодам Unicode
        constexpr std::array<uint16_t, 128> CP1251_LOOKUP = {
            0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021, // 0x80-0x87
            0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F, // 0x88-0x8F (€, ‰...)
            0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, // 0x90-0x97 (тире, кавычки)
            0xFFFD, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F, // 0x98-0x9F (0x98 не определен -> U+FFFD)
            0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7, // 0xA0-0xA7 (0xA1=Ў, 0xA2=ў)
            0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407, // 0xA8-0xAF (0xA8=Ё, 0xAB=«)
            0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7, // 0xB0-0xB7 (0xB1=±, 0xB2=І, 0xB3=і)
            0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457, // 0xB8-0xBF (0xB8=ё, 0xB9=№, 0xBB=»)
            0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, // 0xC0-0xC7 (А-З)
            0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F, // 0xC8-0xCF (И-П)
            0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, // 0xD0-0xD7 (Р-Ч)
            0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F, // 0xD8-0xDF (Ш-Я)
            0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, // 0xE0-0xE7 (а-з)
            0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F, // 0xE8-0xEF (и-п)
            0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, // 0xF0-0xF7 (р-ч)
            0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F  // 0xF8-0xFF (ш-я)
        };

    };// namespace
    std::string cp1251ToUtf8(std::string_view text) {
        std::string result;
        // Предвыделяем память: в худшем случае каждый байт станет 2 или 3 байтами UTF-8
        result.reserve(text.size() * 2);

        for (char ch : text) {
            auto byte = static_cast<uint8_t>(ch);

            if (byte < 0x80) {
                // ASCII (0..127) копируем как есть
                result.push_back(ch);
            }
            else {
                // Идем в нашу таблицу (0x80..0xFF)
                uint16_t unicode = CP1251_LOOKUP[byte - 0x80];

                if (unicode < 0x0800) {
                    // 2 байта UTF-8
                    result.push_back(static_cast<char>(0xC0 | (unicode >> 6)));
                    result.push_back(static_cast<char>(0x80 | (unicode & 0x3F)));
                } else {
                    // 3 байта UTF-8
                    result.push_back(static_cast<char>(0xE0 | (unicode >> 12)));
                    result.push_back(static_cast<char>(0x80 | ((unicode >> 6) & 0x3F)));
                    result.push_back(static_cast<char>(0x80 | (unicode & 0x3F)));
                }
            }
        }

        return result;
    }

    bool isValidUtf8(std::string_view text) noexcept {
        int needed = 0;
        for (char ch : text) {
            auto byte = static_cast<uint8_t>(ch);
            if (needed == 0) {
                if (byte < 0x80) {continue;}
                if ((byte & 0xE0) == 0xC0) {
                    if (byte < 0xC2) return false;
                    needed = 1;
                }
                else if ((byte & 0xF0) == 0xE0) {
                    needed = 2;

                }
                else if ((byte & 0xF8) == 0xF0) {
                    needed = 3;
                }
                else return false;
            }
            else {
                if ((byte & 0xC0) != 0x80) return false;
                else needed--;
            }
        }
        if (needed > 0) return false;
        return true;
    }

    std::string ensureUtf8(std::string_view text) {
        if (isValidUtf8(text)) {
            return std::string(text);
        }
        return cp1251ToUtf8(text);
    }
}

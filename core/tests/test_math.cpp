#include "../src/utils/Int128Math.hpp"
#include "../include/UuidV7.hpp"
#include <iostream>
#include <cassert>
#include <set>


void test_rounding() {
    assert(finance::multiplyDivide(5, 1, 2) == 2);
    assert(finance::multiplyDivide(7, 1, 2) == 4);
    assert(finance::multiplyDivide(10050, 3350000, 1000000) == 33668);
    std::cout << "Rounding tests passed!" << std::endl;
}


void test_currency_conversion() {
    assert(finance::convertCurrency(10000, 3350000, 0, true) == 33500);
    assert(finance::convertCurrency(10000, 3350000, 150, true) == 34002);
    std::cout << "Currency conversion tests passed!" << std::endl;
}


void test_uuid_v7() {
    std::string id1 = finance::generateUuidV7();
    std::string id2 = finance::generateUuidV7();
    std::cout << "Generated UUID 1: " << id1 << std::endl;
    std::cout << "Generated UUID 2: " << id2 << std::endl;
    assert(!id1.empty());
    assert(id1 != id2); // Проверка на уникальность
    std::cout << "UUIDv7 tests passed!" << std::endl;
}


int main() {
    test_rounding();
    test_currency_conversion();
    test_uuid_v7();
    return 0;
}
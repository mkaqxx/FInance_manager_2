#include "UuidV7.hpp"
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

namespace  finance {
    std::string generateUuidV7() noexcept {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<uint64_t> dist;
        uint64_t random_data = dist(gen);

        std::stringstream ss;
        ss << std::hex << std::setfill('0')
           << std::setw(12) << (ms & 0xFFFFFFFFFFFF) // 48 бит времени
           << "7" << std::setw(3) << (random_data & 0xFFF) // Версия 7
           << "-" << std::setw(4) << ((random_data >> 12) & 0xFFFF)
           << "-" << std::setw(12) << (random_data >> 28);

        return ss.str();

    }
}
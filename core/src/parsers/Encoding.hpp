#pragma once

#include <string>
#include <string_view>

namespace finance::parsers {

    /// Проверяет, является ли переданная последовательность байт валидным UTF-8
    [[nodiscard]] bool isValidUtf8(std::string_view text) noexcept;

    /// Декодирует строку из Windows-1251 в UTF-8
    [[nodiscard]] std::string cp1251ToUtf8(std::string_view text);

    /// Гарантирует UTF-8: если уже валидный UTF-8 — возвращает копию, иначе декодирует из CP1251
    [[nodiscard]] std::string ensureUtf8(std::string_view text);

} // namespace finance::parsers
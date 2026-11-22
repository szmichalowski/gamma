#pragma once

#include <cmath>
#include <gamma/lexer/token.hpp>
#include <print>

namespace gm::utils
{

inline constexpr const char *color_red { "\e[1;31m" };
inline constexpr const char *color_reset { "\e[0m" };

namespace
{

inline std::size_t line_width(std::string_view line)
{
    const auto r { line.find('\n') };
    return r == std::string_view::npos ? line.size() : r;
}

inline std::size_t num_of_digits(std::size_t x)
{
    if (x > 0) [[likely]]
    {
        return static_cast<std::size_t>(std::log10(static_cast<double>(x)) + 1);
    }
    return 1;
}

}

template<typename... Args>
void print_error(std::size_t line, std::size_t column, std::size_t width, const char *source_file, const char *line_start, std::format_string<Args...> fmt, Args &&...args)
{
    std::print(stderr, "{}:{}:{}: {}error:{} ", source_file, line, column, color_red, color_reset);
    std::println(stderr, fmt, std::forward<decltype(args)>(args)...);
    std::println(stderr, "    {} | {}{}{}{}", line, std::string_view { line_start, column }, color_red, std::string_view { line_start + column, width }, color_reset, std::string_view { line_start + column + width, line_width(line_start) - column - width });
    std::println(stderr, "{}|{}{}^{}{}", std::string(4 + num_of_digits(line) + 1, ' '), std::string(column, ' '), color_red, std::string(width - 1, '~'), color_reset);
}

}

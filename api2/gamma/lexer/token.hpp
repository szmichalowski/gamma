#pragma once

#include <cstddef>
#include <cstdint>
#include <format>
#include <gamma/lexer/error.hpp>
#include <string>

namespace gm::lexer
{

enum class token_type
{
    identifier,
    number,

    fn,
    ret,

    plus,
    minus,
    star,
    slash,

    equals,
    question,
    grave,
    tilde,
    semicolon,

    lparen,
    rparen,
    lcurly,
    rcurly,
    lsquare,
    rsquare,

    count
};

class token
{
public:
    token(token_type type, std::size_t line, std::size_t column, std::size_t width, const char *source_file, const char *line_start);
    ~token();
    token(const token &) = delete;
    token &operator=(const token &) = delete;
    token(token &&) = delete;
    token &operator=(token &&) = delete;

    token_type &type();
    const token_type &type() const;

    std::size_t &width();
    const std::size_t &width() const;

    std::string &string();
    const std::string &string() const;

    std::int32_t &number();
    const std::int32_t &number() const;

protected:
    token_type m_type;

    const std::size_t m_line;
    const std::size_t m_column;
    std::size_t m_width;

    const char *const m_source_file;
    const char *const m_line_start;

    union
    {
        // token_type::identifier
        std::string m_string;

        // token_type::number
        std::int32_t m_number;
    };

    friend struct std::formatter<gm::lexer::token>;
};

}

template<>
struct std::formatter<gm::lexer::token>
{
    constexpr auto parse(std::format_parse_context &ctx)
    {
        auto it { ctx.begin() };
        if (it != ctx.end() && *it != '}')
        {
            throw std::format_error { "gm::lexer::token accepts only empty formatter '{}'" };
        }
        return it;
    }

    template<typename FormatContext>
    auto format(const gm::lexer::token &token, FormatContext &ctx) const
    {
        static_assert(static_cast<int>(gm::lexer::token_type::count) == 19);
        auto out { std::format_to(ctx.out(), "{}:{}: ", token.m_line, token.m_column) };
        switch (token.m_type)
        {
        case gm::lexer::token_type::identifier:
            return std::format_to(out, "IDENTIFIER: {}", token.m_string);
        case gm::lexer::token_type::number:
            return std::format_to(out, "NUMBER: {}", token.m_number);
        case gm::lexer::token_type::fn:
            return std::format_to(out, "FN");
        case gm::lexer::token_type::ret:
            return std::format_to(out, "RETURN");
        case gm::lexer::token_type::plus:
            return std::format_to(out, "PLUS");
        case gm::lexer::token_type::minus:
            return std::format_to(out, "MINUS");
        case gm::lexer::token_type::star:
            return std::format_to(out, "STAR");
        case gm::lexer::token_type::slash:
            return std::format_to(out, "SLASH");
        case gm::lexer::token_type::equals:
            return std::format_to(out, "EQUALS");
        case gm::lexer::token_type::question:
            return std::format_to(out, "QUESTION");
        case gm::lexer::token_type::grave:
            return std::format_to(out, "GRAVE");
        case gm::lexer::token_type::tilde:
            return std::format_to(out, "TILDE");
        case gm::lexer::token_type::semicolon:
            return std::format_to(out, "SEMICOLON");
        case gm::lexer::token_type::lparen:
            return std::format_to(out, "LEFT PARENTHESIS");
        case gm::lexer::token_type::rparen:
            return std::format_to(out, "RIGHT PARENTHESIS");
        case gm::lexer::token_type::lcurly:
            return std::format_to(out, "LEFT CURLY");
        case gm::lexer::token_type::rcurly:
            return std::format_to(out, "RIGHT CURLY");
        case gm::lexer::token_type::lsquare:
            return std::format_to(out, "LEFT SQUARE");
        case gm::lexer::token_type::rsquare:
            return std::format_to(out, "RIGHT SQUARE");
        default:
            throw gm::lexer::lexer_error {};
        }
    }
};

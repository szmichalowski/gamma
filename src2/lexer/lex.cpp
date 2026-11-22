#include <gamma/lexer/lex.hpp>
#include <gamma/utils/print.hpp>

namespace
{

gm::lexer::token_type char_to_token_type(char c)
{
    static_assert(static_cast<int>(gm::lexer::token_type::count) == 19);

    switch (c)
    {
    case '+':
        return gm::lexer::token_type::plus;
    case '-':
        return gm::lexer::token_type::minus;
    case '*':
        return gm::lexer::token_type::star;
    case '/':
        return gm::lexer::token_type::slash;
    case '=':
        return gm::lexer::token_type::equals;
    case '?':
        return gm::lexer::token_type::question;
    case '`':
        return gm::lexer::token_type::grave;
    case ';':
        return gm::lexer::token_type::semicolon;
    case '(':
        return gm::lexer::token_type::lparen;
    case ')':
        return gm::lexer::token_type::rparen;
    case '{':
        return gm::lexer::token_type::lcurly;
    case '}':
        return gm::lexer::token_type::rcurly;
    case '[':
        return gm::lexer::token_type::lsquare;
    case ']':
        return gm::lexer::token_type::rsquare;
    default:
        throw gm::lexer::lexer_error {};
    }
}

}

namespace gm::lexer
{

std::list<token> lex(std::string_view code, std::string_view path)
{
    static_assert(static_cast<int>(token_type::count) == 19);

    std::list<token> result;

    const char *ch = code.begin();
    const char *line_start = code.begin();
    std::size_t line = 1;
    std::size_t column = 1;
    while (*ch != 0)
    {
        if (*ch == '\n')
        {
            ++line;
            line_start = ch + 1;
            column = 1;
            ++ch;
            continue;
        }

        if (std::isspace(*ch))
        {
            ++column;
            ++ch;
            continue;
        }

        if (*ch == '~')
        {
            if (ch[1] == '~')
            {
                if (ch[2] == '~')
                {
                    const std::size_t saved_line = line;
                    const std::size_t saved_column = column;
                    const char *const saved_line_start = line_start;
                    ch += 3;
                    while (*ch != 0 && (*ch != '~' || ch[1] != '~' || ch[2] != '~'))
                    {
                        if (*ch == '\n')
                        {
                            ++line;
                            line_start = ch + 1;
                            column = 1;
                        }
                        else
                        {
                            ++column;
                        }

                        ++ch;
                    }
                    if (*ch != 0)
                    {
                        column += 3;
                        ch += 3;
                    }
                    else
                    {
                        utils::print_error(saved_line, saved_column, 3, path.begin(), saved_line_start, "multi line comment opened, but not closed");
                        throw lexer_error {};
                    }
                    continue;
                }
                else
                {
                    ch += 2;
                    while (*ch != 0 && *ch != '\n')
                    {
                        ++ch;
                    }
                    if (*ch == '\n')
                    {
                        ++ch;
                        ++line;
                        column = 1;
                        line_start = ch;
                    }
                    continue;
                }
            }
            else
            {
                result.emplace_back(token_type::tilde, line, column, 1, path.begin(), line_start);
                ++column;
                ++ch;
                continue;
            }
        }

        switch (*ch)
        {
        case '+':
        case '-':
        case '*':
        case '/':
        case '=':
        case '?':
        case '`':
        case ';':
        case '(':
        case ')':
        case '{':
        case '}':
        case '[':
        case ']':
            result.emplace_back(::char_to_token_type(*ch), line, column, 1, path.begin(), line_start);
            ++column;
            ++ch;
            continue;
        }

        if (!std::isalnum(*ch) && *ch != '_')
        {
            gm::utils::print_error(line, column, 1, path.begin(), line_start, "character \"{}\" ({:#x}) is not allowed", *ch, *ch);
            throw lexer_error {};
        }

        if (std::isdigit(*ch))
        {
            result.emplace_back(token_type::number, line, column, 1, path.begin(), line_start);
            result.back().number() = *ch - '0';
            ++ch;

            while (*ch != 0)
            {
                if (std::isdigit(*ch))
                {
                    result.back().number() *= 10;
                    result.back().number() += *ch - '0';
                    ++result.back().width();
                    ++column;
                    ++ch;
                    continue;
                }

                if (std::isalpha(*ch))
                {
                    gm::utils::print_error(line, column + 1, 1, path.begin(), line_start, "character \"{}\" after a digit is not allowed", *ch, *ch);
                    throw lexer_error {};
                }

                break;
            }

            ++column;
            continue;
        }

        if (std::isalpha(*ch) || *ch == '_')
        {
            result.emplace_back(token_type::identifier, line, column, 1, path.begin(), line_start);
            result.back().string() += *ch;
            ++ch;

            while (*ch != 0)
            {
                if (std::isalpha(*ch) || std::isdigit(*ch) || *ch == '_')
                {
                    result.back().string() += *ch;
                    ++result.back().width();
                    ++column;
                    ++ch;
                    continue;
                }

                break;
            }

            if (result.back().string() == "fn")
            {
                result.back().type() = token_type::fn;
            }
            else if (result.back().string() == "return")
            {
                result.back().type() = token_type::ret;
            }

            ++column;
            continue;
        }

        throw lexer_error {};
    }

    return result;
}

}

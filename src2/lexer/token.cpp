#include <gamma/lexer/token.hpp>
#include <stdexcept>

namespace gm::lexer
{

token::token(token_type type, std::size_t line, std::size_t column, std::size_t width, const char *source_file, const char *line_start)
    : m_type { type }
    , m_line { line }
    , m_column { column }
    , m_width { width }
    , m_source_file { source_file }
    , m_line_start { line_start }
{
}

token::~token()
{
}

token_type &token::type()
{
    return m_type;
}

const token_type &token::type() const
{
    return m_type;
}

std::size_t &token::width()
{
    return m_width;
}

const std::size_t &token::width() const
{
    return m_width;
}

std::string &token::string()
{
    if (m_type != token_type::identifier)
    {
        throw std::runtime_error { "gm::lexer::token::string(): unreachable code" };
    }

    return m_string;
}

const std::string &token::string() const
{
    if (m_type != token_type::identifier)
    {
        throw std::runtime_error { "gm::lexer::token::string(): unreachable code" };
    }

    return m_string;
}

std::int32_t &token::number()
{
    if (m_type != token_type::number)
    {
        throw std::runtime_error { "gm::lexer::token::number(): unreachable code" };
    }

    return m_number;
}

const std::int32_t &token::number() const
{
    if (m_type != token_type::number)
    {
        throw std::runtime_error { "gm::lexer::token::number(): unreachable code" };
    }

    return m_number;
}

}

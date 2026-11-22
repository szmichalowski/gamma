#include <format>
#include <gamma/lexer/error.hpp>

namespace gm::lexer
{

lexer_error::lexer_error(std::source_location loc)
    : m_loc { loc }
{
}

const char *lexer_error::what() const noexcept
{
    m_msg = std::format("lexer error: {}:{}:{}:{}", m_loc.file_name(), m_loc.line(), m_loc.column(), m_loc.function_name());
    return m_msg.data();
}

}

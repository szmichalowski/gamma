#pragma once

#include <exception>
#include <source_location>
#include <string>

namespace gm::lexer
{

class lexer_error : public std::exception
{
public:
    lexer_error(std::source_location loc = std::source_location::current());
    const char *what() const noexcept override;

protected:
    std::source_location m_loc;
    mutable std::string m_msg;
};

}

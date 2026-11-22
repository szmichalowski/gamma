#pragma once

#include <gamma/lexer/token.hpp>
#include <list>
#include <string_view>

namespace gm::lexer
{

std::list<token> lex(std::string_view code, std::string_view path);

}

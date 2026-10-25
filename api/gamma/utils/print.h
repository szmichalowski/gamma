#ifndef API_UTILS_PRINT_H
#define API_UTILS_PRINT_H

#include <gamma/arena/arena.h>
#include <gamma/lexer/lexer.h>
#include <stddef.h>

[[gnu::format(printf, 7, 8)]] int gm_print_syntax_error(gm_arena_t *arena, size_t line, size_t column, size_t width, const char *source_file, const char *line_start, const char *format, ...);
[[gnu::format(printf, 3, 4)]] int gm_print_syntax_error_token(gm_arena_t *arena, const gm_token_t *token, const char *format, ...);

#endif // API_UTILS_PRINT_H

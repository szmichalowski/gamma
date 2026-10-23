#ifndef API_UTILS_PRINT_H
#define API_UTILS_PRINT_H

#include <gamma/arena/arena.h>
#include <stddef.h>

[[gnu::format(printf, 4, 5)]] int gm_print_syntax_error(gm_arena_t *arena, size_t line, size_t column, const char *format, ...);

#endif // API_UTILS_PRINT_H

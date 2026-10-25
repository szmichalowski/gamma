#include <gamma/utils/print.h>
#include <stdarg.h>
#include <stdio.h>

int gm_print_syntax_error(gm_arena_t *arena, const size_t line, const size_t column, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    const int size = vsnprintf(nullptr, 0, format, args);
    va_end(args);

    if (size < 0)
    {
        return size;
    }

    gm_arena_checkpoint_t checkpoint = gm_arena_checkpoint_begin(arena);

    char *buf = gm_arena_alloc_array(arena, char, size + 1);

    va_start(args, format);
    const int new_size = vsnprintf(buf, size + 1, format, args);
    va_end(args);

    if (new_size != size)
    {
        gm_arena_checkpoint_end(&checkpoint);
        return new_size;
    }

    const int res = fprintf(stderr, "%zu:%zu: Syntax Error: %s\n", line, column, buf);

    gm_arena_checkpoint_end(&checkpoint);

    return res;
}

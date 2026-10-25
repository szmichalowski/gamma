#include <gamma/utils/print.h>
#include <stdarg.h>
#include <stdio.h>

static size_t gm_get_line_width(const char *line)
{
    size_t res = 0;
    while (*line != '\n' && *line != 0)
    {
        ++res;
        ++line;
    }
    return res;
}

#define GM_RED "\e[1;31m"
#define GM_RESET "\e[0m"

static int gm_print_syntax_error_va(gm_arena_t *arena, const size_t line, const size_t column, const size_t width, const char *source_file, const char *line_start, const char *format, va_list args)
{
    va_list args2;
    va_copy(args2, args);

    const int size = vsnprintf(nullptr, 0, format, args);

    if (size < 0)
    {
        return size;
    }

    gm_arena_checkpoint_t checkpoint = gm_arena_checkpoint_begin(arena);

    char *buf = gm_arena_alloc_array(arena, char, size + 1);

    const int new_size = vsnprintf(buf, size + 1, format, args2);
    va_end(args2);

    if (new_size != size)
    {
        gm_arena_checkpoint_end(&checkpoint);
        return new_size;
    }

    int res = fprintf(stderr, "%s:%zu:%zu: " GM_RED "error:" GM_RESET " %s\n", source_file, line, column, buf);
    const int pad = fprintf(stderr, "    %zu ", line);
    res += pad;
    const size_t linew = gm_get_line_width(line_start);
    res += fprintf(stderr, "| ");
    size_t line_counter = 1;
    const char *line_char = line_start;
    while (line_counter < column)
    {
        fputc(*line_char, stderr);
        ++res;
        ++line_counter;
        ++line_char;
    }
    res += fprintf(stderr, GM_RED);
    for (size_t w = 0; w < width; ++w)
    {
        fputc(*line_char, stderr);
        ++res;
        ++line_counter;
        ++line_char;
    }
    res += fprintf(stderr, GM_RESET);
    while (line_counter <= linew)
    {
        fputc(*line_char, stderr);
        ++res;
        ++line_counter;
        ++line_char;
    }
    fputc('\n', stderr);
    ++res;
    for (int i = 0; i < pad; ++i)
    {
        fputc(' ', stderr);
        ++res;
    }
    fputc('|', stderr);
    ++res;
    for (size_t i = 0; i < column; ++i)
    {
        fputc(' ', stderr);
        ++res;
    }
    res += fprintf(stderr, GM_RED);
    fputc('^', stderr);
    ++res;
    for (size_t i = 1; i < width; ++i)
    {
        fputc('~', stderr);
        ++res;
    }
    res += fprintf(stderr, GM_RESET "\n");

    gm_arena_checkpoint_end(&checkpoint);

    return res;
}

int gm_print_syntax_error(gm_arena_t *arena, const size_t line, const size_t column, const size_t width, const char *source_file, const char *line_start, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    const int size = gm_print_syntax_error_va(arena, line, column, width, source_file, line_start, format, args);
    va_end(args);

    return size;
}
int gm_print_syntax_error_token(gm_arena_t *arena, const gm_token_t *token, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    const int size = gm_print_syntax_error_va(arena, token->line, token->column, token->width, token->source_file, token->line_start, format, args);
    va_end(args);

    return size;
}

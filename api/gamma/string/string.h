#ifndef GM_API_STRING_STRING_H
#define GM_API_STRING_STRING_H

#include <gamma/arena/arena.h>

typedef struct gm_string
{
    char *data;
    size_t size;
} gm_string_t;

void gm_string_init(gm_arena_t *arena, gm_string_t *string);
void gm_string_append_char(gm_arena_t *arena, gm_string_t *string, char ch);

#endif // GM_API_STRING_STRING_H

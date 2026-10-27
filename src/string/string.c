#include <gamma/string/string.h>
#include <string.h>

void gm_string_init(gm_arena_t *arena, gm_string_t *string)
{
    // gm_arena_alloc_array() zeroes out the memory on success
    string->data = gm_arena_alloc_array(arena, char, 1);
    string->size = 0;
}

void gm_string_append_char(gm_arena_t *arena, gm_string_t *string, char ch)
{
    // gm_arena_realloc_array() zeroes out the new memory on success
    string->data = gm_arena_realloc_array(arena, char, string->data, string->size + 1, string->size + 2);
    string->data[string->size++] = ch;
}

void gm_string_append_string(gm_arena_t *arena, gm_string_t *string, const gm_string_t other)
{
    string->data = gm_arena_realloc_array(arena, char, string->data, string->size + 1, string->size + 1 + other.size);
    for (char *it = other.data; *it != 0; ++it)
    {
        string->data[string->size++] = *it;
    }
}

void gm_string_append_strlit(gm_arena_t *arena, gm_string_t *string, const char *other)
{
    string->data = gm_arena_realloc_array(arena, char, string->data, string->size + 1, string->size + 1 + strlen(other));
    for (const char *it = other; *it != 0; ++it)
    {
        string->data[string->size++] = *it;
    }
}

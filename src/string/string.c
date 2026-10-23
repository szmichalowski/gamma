#include <gamma/string/string.h>

bool gm_string_init(gm_arena_t *arena, gm_string_t *string)
{
    // gm_arena_alloc_array() zeroes out the memory on success
    string->data = gm_arena_alloc_array(arena, char, 1);
    string->size = 0;
    return string->data != nullptr;
}

bool gm_string_append_char(gm_arena_t *arena, gm_string_t *string, char ch)
{
    // gm_arena_realloc_array() zeroes out the new memory on success
    char *new_ptr = gm_arena_realloc_array(arena, char, string->data, string->size + 1, string->size + 2);
    if (new_ptr == nullptr)
    {
        return false;
    }

    string->data = new_ptr;
    string->data[string->size++] = ch;
    return true;
}

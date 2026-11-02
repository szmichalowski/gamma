#ifndef GM_PARSER_TYPE_H
#define GM_PARSER_TYPE_H

#include <gamma/lexer/lexer.h>
#include <gamma/utils/arena.h>
#include <stdint.h>

typedef enum gm_type_kind
{
    GM_TYPE_KIND_S32,

    GM_TYPE_KIND_GUARDIAN_ANGEL
} gm_type_kind_e;

typedef struct gm_type
{
    gm_type_kind_e kind;
    bool is_mutable;
} gm_type_t;

/*typedef struct gm_value
{
    gm_type_kind_e kind;
    union
    {
        // GM_TYPE_KIND_S32
        struct
        {
            int32_t value;
        } s32;
    };
} gm_value_t;*/

bool gm_parse_type(gm_arena_t *arena, const gm_token_t *tokens, gm_type_t *type, const gm_token_t **out);

bool gm_can_type_mutate(gm_type_t type);
bool gm_can_type_be_returned_from_a_function(gm_type_t type);

const char *gm_type_to_string(gm_type_t type);

#endif // GM_PARSER_TYPE_H

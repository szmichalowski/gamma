#ifndef API_PARSER_STATEMENT_H
#define API_PARSER_STATEMENT_H

#include <gamma/arena/arena.h>
#include <gamma/lexer/lexer.h>
#include <stdint.h>

typedef enum gm_type
{
    GM_TYPE_S32,

    GM_TYPE_GUARDIAN_ANGEL
} gm_type_e;

typedef enum gm_statement_type
{
    GM_STATEMENT_TYPE_VAR_DECL,

    GM_STATEMENT_TYPE_RETURN,

    GM_STATEMENT_TYPE_GUARDIAN_ANGEL
} gm_statement_type_e;

typedef struct gm_statement gm_statement_t;
struct gm_statement
{
    gm_statement_t *next;

    gm_statement_type_e type;
    union
    {
        // GM_STATEMENT_TYPE_VAR_DECL
        struct
        {
            const gm_token_t *token_name;
            const gm_token_t *token_type;
            gm_string_t name;
            gm_type_e type;
            int32_t number;
            bool initialized;
        } var_decl;

        // GM_STATEMENT_TYPE_RETURN
        struct
        {
            const gm_token_t *token_return;
            const gm_token_t *token_number;
            int32_t number;
        } ret;
    };
};

bool gm_parse_fn_body(gm_arena_t *arena, const gm_token_t *tokens, gm_statement_t **statements, const gm_token_t **out);

bool gm_ensure_fn_integrity(gm_arena_t *arena, const gm_token_t *fn_token, const gm_statement_t *statements);

const char *gm_type_to_string(const gm_type_e type);
void gm_print_statements(const gm_statement_t *statements);

#endif // API_PARSER_STATEMENT_H

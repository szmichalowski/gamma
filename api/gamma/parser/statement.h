#ifndef API_PARSER_STATEMENT_H
#define API_PARSER_STATEMENT_H

#include <gamma/arena/arena.h>
#include <gamma/lexer/lexer.h>
#include <gamma/parser/expression.h>
#include <gamma/parser/type.h>

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
            gm_expression_node_t *expression;
            bool initialized;
        } var_decl;

        // GM_STATEMENT_TYPE_RETURN
        struct
        {
            const gm_token_t *token_return;
            gm_expression_node_t *expression;
        } ret;
    };
};

bool gm_parse_fn_body(gm_arena_t *arena, const gm_token_t *tokens, gm_statement_t **statements, const gm_token_t **out);

bool gm_ensure_fn_integrity(gm_arena_t *arena, const gm_token_t *fn_token, const gm_statement_t *statements);

void gm_print_statements(const gm_statement_t *statements);

#endif // API_PARSER_STATEMENT_H

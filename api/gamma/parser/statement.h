#ifndef API_PARSER_STATEMENT_H
#define API_PARSER_STATEMENT_H

#include <gamma/lexer/lexer.h>
#include <gamma/parser/expression.h>
#include <gamma/parser/type.h>
#include <gamma/utils/arena.h>

typedef enum gm_statement_type
{
    GM_STATEMENT_TYPE_VAR_DECL,
    GM_STATEMENT_TYPE_VAR_ASSIGN,

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
            gm_string_t name;
            gm_type_t type;
            gm_expression_node_t *expression;
            bool initialized;
            bool used;
            bool modified;
        } var_decl;

        // GM_STATEMENT_TYPE_VAR_ASSIGN
        struct
        {
            const gm_statement_t *declaration;
            const gm_token_t *token_name;
            gm_string_t name;
            gm_expression_node_t *expression;
        } var_assign;

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

typedef struct gm_fn_parse_ctx
{
    gm_statement_t **var_decls;
    size_t var_decls_size;
    size_t var_decls_cap;
} gm_fn_parse_ctx_t;

gm_fn_parse_ctx_t gm_fn_parse_ctx_new(gm_arena_t *arena);

void gm_fn_parse_ctx_add_var_decl(gm_arena_t *arena, gm_fn_parse_ctx_t *ctx, gm_statement_t *decl);
gm_statement_t *gm_fn_parse_ctx_find_var_decl(gm_fn_parse_ctx_t *ctx, const char *var_name);

#endif // API_PARSER_STATEMENT_H

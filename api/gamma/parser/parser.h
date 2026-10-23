#ifndef API_PARSER_PARSER_H
#define API_PARSER_PARSER_H

#include "gamma/arena/arena.h"
#include <gamma/generator/generator.h>
#include <gamma/lexer/lexer.h>
#include <gamma/parser/statement.h>
#include <gamma/string/string.h>

typedef enum gm_type
{
    GM_TYPE_S32,

    GM_TYPE_GUARDIAN_ANGEL
} gm_type_e;

typedef enum gm_ast_node_type
{
    GM_AST_NODE_TYPE_FN,

    GM_AST_NODE_TYPE_GUARDIAN_ANGEL
} gm_ast_node_type_e;

typedef struct gm_ast_fn
{
    gm_string_t name;
    gm_type_e return_type;

    gm_statement_t *statements;
} gm_ast_fn_t;

typedef struct gm_ast_node gm_ast_node_t;
struct gm_ast_node
{
    gm_ast_node_t *next;

    gm_ast_node_type_e type;
    union
    {
        // GM_AST_NODE_TYPE_FN
        struct
        {
            gm_ast_fn_t fn;
            gm_amd64_instruction_t *instructions;
        };
    };
};

bool gm_parse_tokens(gm_arena_t *arena, const gm_token_t *tokens, gm_ast_node_t **out);

bool gm_ensure_ast_integrity(gm_arena_t *arena, const gm_ast_node_t *ast_tree);

bool gm_compile_amd64_ast_tree(gm_arena_t *arena, gm_ast_node_t *ast_tree);

void gm_print_ast_tree(const gm_ast_node_t *ast_tree);
void gm_print_amd64_ast_tree(const gm_ast_node_t *ast_tree);

#endif // API_PARSER_PARSER_H

#ifndef API_PARSER_EXPRESSION_H
#define API_PARSER_EXPRESSION_H

#include <gamma/arena/arena.h>
#include <gamma/lexer/lexer.h>
#include <stdint.h>

typedef enum gm_expression_node_type
{
    GM_EXPRESSION_NODE_TYPE_LEAF,

    GM_EXPRESSION_NODE_TYPE_ADDITION,
    GM_EXPRESSION_NODE_TYPE_SUBTRACTION,
    GM_EXPRESSION_NODE_TYPE_MULTIPLICATION,
    GM_EXPRESSION_NODE_TYPE_DIVISION,

    GM_EXPRESSION_NODE_TYPE_GUARDIAN_ANGEL
} gm_expression_node_type_e;

typedef struct gm_expression_node gm_expression_node_t;
struct gm_expression_node
{
    gm_expression_node_t *parent;

    gm_expression_node_type_e type;
    union
    {
        // GM_EXPRESSION_NODE_TYPE_ADDITION
        // GM_EXPRESSION_NODE_TYPE_SUBTRACTION
        // GM_EXPRESSION_NODE_TYPE_MULTIPLICATION
        // GM_EXPRESSION_NODE_TYPE_DIVISION
        struct
        {
            gm_expression_node_t *left;
            gm_expression_node_t *right;
            const gm_token_t *token_op;
        } op;

        // GM_EXPRESSION_NODE_TYPE_LEAF
        struct
        {
            const gm_token_t *token_number;
            int32_t number;
        } leaf;
    };
};

bool gm_parse_expression(gm_arena_t *arena, const gm_token_t *tokens, gm_expression_node_t **expression, const gm_token_t **out);

gm_expression_node_t *gm_expression_zero(gm_arena_t *arena);

void gm_print_expression(const gm_expression_node_t *expression, size_t pad);

#endif // API_PARSER_EXPRESSION_H

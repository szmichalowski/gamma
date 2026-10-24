#include <gamma/arena/arena.h>
#include <gamma/lexer/lexer.h>
#include <gamma/parser/parser.h>
#include <gamma/utils/print.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

bool gm_parse_tokens(gm_arena_t *arena, const gm_token_t *tokens, gm_ast_node_t **out)
{
    static_assert(GM_TOKEN_TYPE_GUARDIAN_ANGEL == 11, "gm_parse_tokens(): token type enum size changed");
    static_assert(GM_TYPE_GUARDIAN_ANGEL == 1, "gm_parse_tokens(): type enum size changed");
    static_assert(GM_STATEMENT_TYPE_GUARDIAN_ANGEL == 1, "gm_parse_tokens(): statement type enum size changed");

    // gm_arena_alloc_single() zeroes out allocated memory on success, so result-<next is a nullptr
    gm_ast_node_t *result = gm_arena_alloc_single(arena, gm_ast_node_t);
    if (result == nullptr)
    {
        fputs("ERROR: gm_parse_tokens(): allocation error\n", stderr);
        return false;
    }
    gm_ast_node_t *it = result;

    while (tokens != nullptr)
    {
        if (tokens->type != GM_TOKEN_TYPE_FN)
        {
            gm_print_syntax_error(arena, tokens->line, tokens->column, "expected only function declaration in the global scope");
            return false;
        }

        it->next = gm_arena_alloc_single(arena, gm_ast_node_t);
        if (it->next == nullptr)
        {
            fputs("ERROR: gm_parse_tokens(): allocation error\n", stderr);
            return false;
        }
        it = it->next;
        it->type = GM_AST_NODE_TYPE_FN;

        const gm_token_t *name = tokens->next;
        if (name == nullptr)
        {
            gm_print_syntax_error(arena, tokens->line, tokens->column, "unexpected end of file");
            return false;
        }
        if (name->type != GM_TOKEN_TYPE_IDENTIFIER)
        {
            gm_print_syntax_error(arena, name->line, name->column, "expected identifier");
            return false;
        }
        it->fn.name = name->string;

        const gm_token_t *left_paren = name->next;
        if (left_paren == nullptr)
        {
            gm_print_syntax_error(arena, left_paren->line, left_paren->column, "unexpected end of file");
            return false;
        }
        if (left_paren->type != GM_TOKEN_TYPE_LEFT_PAREN)
        {
            gm_print_syntax_error(arena, left_paren->line, left_paren->column, "expected left parenthesis");
            return false;
        }

        const gm_token_t *right_paren = left_paren->next;
        if (right_paren == nullptr)
        {
            gm_print_syntax_error(arena, right_paren->line, right_paren->column, "unexpected end of file");
            return false;
        }
        if (right_paren->type != GM_TOKEN_TYPE_RIGHT_PAREN)
        {
            gm_print_syntax_error(arena, right_paren->line, right_paren->column, "expected right parenthesis");
            return false;
        }

        const gm_token_t *left_square = right_paren->next;
        if (left_square == nullptr)
        {
            gm_print_syntax_error(arena, left_square->line, left_square->column, "unexpected end of file");
            return false;
        }
        if (left_square->type != GM_TOKEN_TYPE_LEFT_SQUARE)
        {
            gm_print_syntax_error(arena, left_square->line, left_square->column, "expected left square bracket");
            return false;
        }

        const gm_token_t *return_type = left_square->next;
        if (return_type == nullptr)
        {
            gm_print_syntax_error(arena, return_type->line, return_type->column, "unexpected end of file");
            return false;
        }
        if (return_type->type != GM_TOKEN_TYPE_IDENTIFIER)
        {
            gm_print_syntax_error(arena, return_type->line, return_type->column, "expected identifier");
            return false;
        }
        if (strcmp(return_type->string.data, "s32") != 0)
        {
            gm_print_syntax_error(arena, return_type->line, return_type->column, "invalid type: \"%s\"", return_type->string.data);
            return false;
        }
        it->fn.return_type = GM_TYPE_S32;

        const gm_token_t *right_square = return_type->next;
        if (right_square == nullptr)
        {
            gm_print_syntax_error(arena, right_square->line, right_square->column, "unexpected end of file");
            return false;
        }
        if (right_square->type != GM_TOKEN_TYPE_RIGHT_SQUARE)
        {
            gm_print_syntax_error(arena, right_square->line, right_square->column, "expected right square bracket");
            return false;
        }

        if (!gm_parse_fn_body(arena, right_square->next, &it->fn.statements, &tokens))
        {
            return false;
        }
    }

    *out = result->next;
    return true;
}

static const char *gm_type_to_string(const gm_type_e type)
{
    static_assert(GM_TYPE_GUARDIAN_ANGEL == 1, "gm_type_to_string(): type enum size changed");

    switch (type)
    {
    case GM_TYPE_S32:
        return "s32";
    default:
        fprintf(stderr, "ERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
        return "<UNREACHABLE>";
    }
}

bool gm_ensure_ast_integrity(gm_arena_t *arena, const gm_ast_node_t *ast_tree)
{
    static_assert(GM_AST_NODE_TYPE_GUARDIAN_ANGEL == 1, "gm_print_ast_tree(): AST node type enum size changed");

    bool res = true;

    for (const gm_ast_node_t *it = ast_tree; it != nullptr; it = it->next)
    {
        switch (it->type)
        {
        case GM_AST_NODE_TYPE_FN:
            const bool fn_ret = gm_ensure_fn_integrity(arena, it->fn.statements);
            if (!fn_ret)
            {
                res = false;
            }
            break;
        default:
            fprintf(stderr, "ERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
            return false;
        }
    }

    return res;
}

void gm_print_ast_tree(const gm_ast_node_t *ast_tree)
{
    if (ast_tree == nullptr)
    {
        puts("<NO AST>");
        return;
    }

    static_assert(GM_AST_NODE_TYPE_GUARDIAN_ANGEL == 1, "gm_print_ast_tree(): AST node type enum size changed");
    for (const gm_ast_node_t *it = ast_tree; it != nullptr; it = it->next)
    {
        switch (it->type)
        {
        case GM_AST_NODE_TYPE_FN:
            printf("FUNCTION \"%s\" that returns \"%s\"\n", it->fn.name.data, gm_type_to_string(it->fn.return_type));
            gm_print_statements(it->fn.statements);
            putchar('\n');
            break;
        default:
            fprintf(stderr, "ERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
            return;
        }
    }
}

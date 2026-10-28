#include <gamma/arena/arena.h>
#include <gamma/lexer/lexer.h>
#include <gamma/parser/parser.h>
#include <gamma/utils/print.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

bool gm_parse_tokens(gm_arena_t *arena, const gm_token_t *tokens, gm_ast_node_t **out)
{
    static_assert(GM_TOKEN_TYPE_GUARDIAN_ANGEL == 12, "gm_parse_tokens(): token type enum size changed");
    static_assert(GM_TYPE_GUARDIAN_ANGEL == 1, "gm_parse_tokens(): type enum size changed");
    static_assert(GM_STATEMENT_TYPE_GUARDIAN_ANGEL == 2, "gm_parse_tokens(): statement type enum size changed");

    // gm_arena_alloc_single() zeroes out allocated memory on success, so result-<next is a nullptr
    gm_ast_node_t *result = gm_arena_alloc_single(arena, gm_ast_node_t);
    gm_ast_node_t *it = result;

    while (tokens != nullptr)
    {
        if (tokens->type != GM_TOKEN_TYPE_FN)
        {
            gm_print_syntax_error_token(arena, tokens, "expected only function declaration in the global scope");
            return false;
        }

        it->next = gm_arena_alloc_single(arena, gm_ast_node_t);
        it = it->next;
        it->type = GM_AST_NODE_TYPE_FN;

        const gm_token_t *name = tokens->next;
        if (name == nullptr)
        {
            gm_print_syntax_error_token(arena, tokens, "unexpected end of file");
            return false;
        }
        if (name->type != GM_TOKEN_TYPE_IDENTIFIER)
        {
            gm_print_syntax_error_token(arena, name, "expected identifier");
            return false;
        }
        it->fn.name = name->string;
        it->fn.token_name = name;

        const gm_token_t *left_paren = name->next;
        if (left_paren == nullptr)
        {
            gm_print_syntax_error_token(arena, left_paren, "unexpected end of file");
            return false;
        }
        if (left_paren->type != GM_TOKEN_TYPE_LEFT_PAREN)
        {
            gm_print_syntax_error_token(arena, left_paren, "expected left parenthesis");
            return false;
        }

        const gm_token_t *right_paren = left_paren->next;
        if (right_paren == nullptr)
        {
            gm_print_syntax_error_token(arena, right_paren, "unexpected end of file");
            return false;
        }
        if (right_paren->type != GM_TOKEN_TYPE_RIGHT_PAREN)
        {
            gm_print_syntax_error_token(arena, right_paren, "expected right parenthesis");
            return false;
        }

        const gm_token_t *grave = right_paren->next;
        if (grave == nullptr)
        {
            gm_print_syntax_error_token(arena, grave, "unexpected end of file");
            return false;
        }
        if (grave->type != GM_TOKEN_TYPE_GRAVE)
        {
            gm_print_syntax_error_token(arena, grave, "expected grave/backtick");
            return false;
        }

        const gm_token_t *return_type = grave->next;
        if (return_type == nullptr)
        {
            gm_print_syntax_error_token(arena, return_type, "unexpected end of file");
            return false;
        }
        if (return_type->type != GM_TOKEN_TYPE_IDENTIFIER)
        {
            gm_print_syntax_error_token(arena, return_type, "expected identifier");
            return false;
        }
        if (strcmp(return_type->string.data, "s32") != 0)
        {
            gm_print_syntax_error_token(arena, return_type, "invalid type: \"%s\"", return_type->string.data);
            return false;
        }
        it->fn.return_type = GM_TYPE_S32;

        if (!gm_parse_fn_body(arena, return_type->next, &it->fn.statements, &tokens))
        {
            return false;
        }
    }

    *out = result->next;
    return true;
}

static bool gm_ensure_fn_unique_names(gm_arena_t *arena, const gm_ast_node_t *ast_tree)
{
    static_assert(GM_AST_NODE_TYPE_GUARDIAN_ANGEL == 1, "gm_ensure_fn_unique_names(): AST node type enum size changed");

    for (const gm_ast_node_t *it = ast_tree; it != nullptr; it = it->next)
    {
        switch (it->type)
        {
        case GM_AST_NODE_TYPE_FN:
            const gm_ast_fn_t *fn = &it->fn;
            for (const gm_ast_node_t *it2 = ast_tree; it2 != nullptr; it2 = it2->next)
            {
                if (&it2->fn != fn && strcmp(it2->fn.name.data, fn->name.data) == 0)
                {
                    gm_print_syntax_error_token(arena, it2->fn.token_name, "function with the name \"%s\" has already been declared", it2->fn.name.data);
                    return false;
                }
            }
            break;
        default:
            fprintf(stderr, "ERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
            return false;
        }
    }

    return true;
}

bool gm_ensure_ast_integrity(gm_arena_t *arena, const gm_ast_node_t *ast_tree)
{
    static_assert(GM_AST_NODE_TYPE_GUARDIAN_ANGEL == 1, "gm_print_ast_tree(): AST node type enum size changed");

    if (!gm_ensure_fn_unique_names(arena, ast_tree))
    {
        return false;
    }

    bool res = true;

    for (const gm_ast_node_t *it = ast_tree; it != nullptr; it = it->next)
    {
        switch (it->type)
        {
        case GM_AST_NODE_TYPE_FN:
            const bool fn_ret = gm_ensure_fn_integrity(arena, it->fn.token_name, it->fn.statements);
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

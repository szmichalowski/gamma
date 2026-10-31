#include <gamma/parser/expression.h>
#include <gamma/utils/print.h>
#include <inttypes.h>
#include <stdio.h>

bool gm_parse_expression(gm_arena_t *arena, const gm_token_t *tokens, gm_expression_node_t **expression, const gm_token_t **out)
{
    static_assert(GM_TOKEN_TYPE_GUARDIAN_ANGEL == 18, "gm_parse_expression(): token type enum size changed");
    static_assert(GM_EXPRESSION_NODE_TYPE_GUARDIAN_ANGEL == 5, "gm_parse_expression(): expression node type enum size changed");

    bool is_negative = false;
    if (tokens->type == GM_TOKEN_TYPE_MINUS)
    {
        is_negative = true;
        if (tokens->next == nullptr)
        {
            gm_print_syntax_error_token(arena, tokens, "unexpected end of file");
            return false;
        }
        tokens = tokens->next;
    }

    gm_expression_node_t *root;
    gm_expression_node_t *newest;
    if (tokens->type == GM_TOKEN_TYPE_LEFT_PAREN)
    {
        if (tokens->next == nullptr)
        {
            gm_print_syntax_error_token(arena, tokens, "unexpected end of file");
            return false;
        }

        if (!gm_parse_expression(arena, tokens->next, &root, &tokens))
        {
            return false;
        }

        if (tokens->type != GM_TOKEN_TYPE_RIGHT_PAREN)
        {
            gm_print_syntax_error_token(arena, tokens, "expected right parenthesis");
            return false;
        }

        newest = root;

        if (is_negative)
        {
            gm_expression_node_t *new_root = gm_arena_alloc_single(arena, gm_expression_node_t);
            new_root->type = GM_EXPRESSION_NODE_TYPE_MULTIPLICATION;
            new_root->op.left = root;
            root->parent = new_root;

            gm_expression_node_t *new_val = gm_arena_alloc_single(arena, gm_expression_node_t);
            new_val->type = GM_EXPRESSION_NODE_TYPE_LEAF;
            new_val->leaf.number = -1;
            new_root->op.right = new_val;
            new_val->parent = new_root;

            root = newest = new_root;
        }
    }
    else if (tokens->type == GM_TOKEN_TYPE_NUMBER)
    {
        root = gm_arena_alloc_single(arena, gm_expression_node_t);
        root->type = GM_EXPRESSION_NODE_TYPE_LEAF;
        root->leaf.token_number = tokens;
        root->leaf.number = is_negative ? -tokens->number : tokens->number;
        newest = root;
    }
    else
    {
        gm_print_syntax_error_token(arena, tokens, "unexpected token");
        return false;
    }

    if (tokens->next == nullptr)
    {
        gm_print_syntax_error_token(arena, tokens, "unexpected end of file");
        return false;
    }
    tokens = tokens->next;

    while (tokens->type == GM_TOKEN_TYPE_PLUS || tokens->type == GM_TOKEN_TYPE_MINUS || tokens->type == GM_TOKEN_TYPE_STAR || tokens->type == GM_TOKEN_TYPE_SLASH)
    {
        if (tokens->next == nullptr)
        {
            gm_print_syntax_error_token(arena, tokens, "unexpected end of file");
            return false;
        }

        if (tokens->next->type != GM_TOKEN_TYPE_NUMBER && tokens->next->type != GM_TOKEN_TYPE_LEFT_PAREN)
        {
            gm_print_syntax_error_token(arena, tokens->next, "unexpected token");
            return false;
        }

        if (tokens->type == GM_TOKEN_TYPE_PLUS || tokens->type == GM_TOKEN_TYPE_MINUS)
        {
            gm_expression_node_t *new_root = gm_arena_alloc_single(arena, gm_expression_node_t);
            new_root->type = tokens->type == GM_TOKEN_TYPE_PLUS ? GM_EXPRESSION_NODE_TYPE_ADDITION : GM_EXPRESSION_NODE_TYPE_SUBTRACTION;
            new_root->op.token_op = tokens;
            new_root->op.left = root;
            root->parent = new_root;
            root = new_root;

            if (tokens->next->type == GM_TOKEN_TYPE_NUMBER)
            {
                gm_expression_node_t *new_val = gm_arena_alloc_single(arena, gm_expression_node_t);
                new_val->type = GM_EXPRESSION_NODE_TYPE_LEAF;
                new_val->leaf.token_number = tokens->next;
                new_val->leaf.number = tokens->next->number;
                new_val->parent = new_root;
                new_root->op.right = newest = new_val;

                if (tokens->next->next == nullptr)
                {
                    gm_print_syntax_error_token(arena, tokens->next, "unexpected end of file");
                    return false;
                }
                tokens = tokens->next->next;
            }
            else
            {
                if (tokens->next->next == nullptr)
                {
                    gm_print_syntax_error_token(arena, tokens->next, "unexpected end of file");
                    return false;
                }
                if (!gm_parse_expression(arena, tokens->next->next, &new_root->op.right, &tokens))
                {
                    return false;
                }
                if (tokens->type != GM_TOKEN_TYPE_RIGHT_PAREN)
                {
                    gm_print_syntax_error_token(arena, tokens, "expected right parenthesis");
                    return false;
                }
                new_root->op.right->parent = new_root;
                newest = new_root->op.right;

                if (tokens->next == nullptr)
                {
                    gm_print_syntax_error_token(arena, tokens, "unexpected end of file");
                    return false;
                }
                tokens = tokens->next;
            }
        }
        else
        {
            gm_expression_node_t *ptr = newest;
            while (ptr->parent != nullptr && (ptr->parent->type == GM_EXPRESSION_NODE_TYPE_MULTIPLICATION || ptr->parent->type == GM_EXPRESSION_NODE_TYPE_DIVISION))
            {
                ptr = ptr->parent;
            }

            gm_expression_node_t *ptr_copy = gm_arena_alloc_single(arena, gm_expression_node_t);
            *ptr_copy = *ptr;
            ptr->type = tokens->type == GM_TOKEN_TYPE_STAR ? GM_EXPRESSION_NODE_TYPE_MULTIPLICATION : GM_EXPRESSION_NODE_TYPE_DIVISION;
            ptr->op.token_op = ptr_copy->op.token_op;
            ptr->op.left = ptr_copy;
            ptr_copy->parent = ptr;

            if (tokens->next->type == GM_TOKEN_TYPE_NUMBER)
            {
                gm_expression_node_t *new_val = gm_arena_alloc_single(arena, gm_expression_node_t);
                new_val->type = GM_EXPRESSION_NODE_TYPE_LEAF;
                new_val->leaf.token_number = tokens->next;
                new_val->leaf.number = tokens->next->number;
                ptr->op.right = new_val;
                new_val->parent = ptr;
                newest = new_val;

                if (tokens->next->next == nullptr)
                {
                    gm_print_syntax_error_token(arena, tokens->next, "unexpected end of file");
                    return false;
                }
                tokens = tokens->next->next;
            }
            else
            {
                if (tokens->next->next == nullptr)
                {
                    gm_print_syntax_error_token(arena, tokens->next, "unexpected end of file");
                    return false;
                }
                if (!gm_parse_expression(arena, tokens->next->next, &ptr->op.right, &tokens))
                {
                    return false;
                }
                if (tokens->type != GM_TOKEN_TYPE_RIGHT_PAREN)
                {
                    gm_print_syntax_error_token(arena, tokens, "expected right parenthesis");
                    return false;
                }
                ptr->op.right->parent = ptr;
                newest = ptr->op.right;

                if (tokens->next == nullptr)
                {
                    gm_print_syntax_error_token(arena, tokens, "unexpected end of file");
                    return false;
                }
                tokens = tokens->next;
            }
        }
    }

    *expression = root;
    *out = tokens;
    return true;
}

gm_expression_node_t *gm_expression_zero(gm_arena_t *arena)
{
    gm_expression_node_t *zero = gm_arena_alloc_single(arena, gm_expression_node_t);
    zero->type = GM_EXPRESSION_NODE_TYPE_LEAF;
    zero->leaf.token_number = nullptr;
    zero->leaf.number = 0;
    return zero;
}

void gm_print_expression(const gm_expression_node_t *expression, const size_t pad)
{
    static_assert(GM_EXPRESSION_NODE_TYPE_GUARDIAN_ANGEL == 5, "gm_parse_expression(): expression node type enum size changed");

    switch (expression->type)
    {
    case GM_EXPRESSION_NODE_TYPE_LEAF:
        for (size_t i = 0; i < pad; ++i)
        {
            putchar(' ');
        }
        printf("%" PRId32 "\n", expression->leaf.number);
        break;
    case GM_EXPRESSION_NODE_TYPE_ADDITION:
        gm_print_expression(expression->op.right, pad + 4);
        for (size_t i = 0; i < pad; ++i)
        {
            putchar(' ');
        }
        puts("+");
        gm_print_expression(expression->op.left, pad + 4);
        break;
    case GM_EXPRESSION_NODE_TYPE_SUBTRACTION:
        gm_print_expression(expression->op.right, pad + 4);
        for (size_t i = 0; i < pad; ++i)
        {
            putchar(' ');
        }
        puts("-");
        gm_print_expression(expression->op.left, pad + 4);
        break;
    case GM_EXPRESSION_NODE_TYPE_MULTIPLICATION:
        gm_print_expression(expression->op.right, pad + 4);
        for (size_t i = 0; i < pad; ++i)
        {
            putchar(' ');
        }
        puts("*");
        gm_print_expression(expression->op.left, pad + 4);
        break;
    case GM_EXPRESSION_NODE_TYPE_DIVISION:
        gm_print_expression(expression->op.right, pad + 4);
        for (size_t i = 0; i < pad; ++i)
        {
            putchar(' ');
        }
        puts("/");
        gm_print_expression(expression->op.left, pad + 4);
        break;
    default:
        fprintf(stderr, "\tERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
        return;
    }
}

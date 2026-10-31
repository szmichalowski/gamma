#include "gamma/parser/expression.h"
#include <gamma/parser/statement.h>
#include <gamma/utils/print.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

bool gm_parse_fn_body(gm_arena_t *arena, const gm_token_t *tokens, gm_statement_t **statements, const gm_token_t **out)
{
    static_assert(GM_TOKEN_TYPE_GUARDIAN_ANGEL == 18, "gm_parse_fn_body(): token type enum size changed");
    static_assert(GM_STATEMENT_TYPE_GUARDIAN_ANGEL == 2, "gm_parse_fn_body(): statement type enum size changed");

    if (tokens->type != GM_TOKEN_TYPE_LEFT_CURLY)
    {
        gm_print_syntax_error_token(arena, tokens, "expected left curly brace");
        return false;
    }
    if (tokens->next == nullptr)
    {
        gm_print_syntax_error_token(arena, tokens, "unexpected end of file");
        return false;
    }
    tokens = tokens->next;

    gm_statement_t *statements_result = gm_arena_alloc_single(arena, gm_statement_t);
    gm_statement_t *statements_it = statements_result;

    while (tokens->type != GM_TOKEN_TYPE_RIGHT_CURLY)
    {
        if (tokens->type == GM_TOKEN_TYPE_RETURN)
        {
            statements_it->next = gm_arena_alloc_single(arena, gm_statement_t);
            statements_it = statements_it->next;
            statements_it->type = GM_STATEMENT_TYPE_RETURN;
            statements_it->ret.token_return = tokens;

            if (tokens->next == nullptr)
            {
                gm_print_syntax_error_token(arena, tokens, "unexpected end of file");
                return false;
            }

            if (!gm_parse_expression(arena, tokens->next, &statements_it->ret.expression, &tokens))
            {
                return false;
            }

            const gm_token_t *semicolon = tokens;
            if (semicolon->type != GM_TOKEN_TYPE_SEMICOLON)
            {
                gm_print_syntax_error_token(arena, semicolon, "expected a semicolon");
                return false;
            }

            tokens = semicolon->next;
            if (tokens == nullptr)
            {
                gm_print_syntax_error_token(arena, semicolon, "unexpected end of file");
                return false;
            }
        }
        else if (tokens->type == GM_TOKEN_TYPE_IDENTIFIER)
        {
            statements_it->next = gm_arena_alloc_single(arena, gm_statement_t);
            statements_it = statements_it->next;
            statements_it->type = GM_STATEMENT_TYPE_VAR_DECL;
            statements_it->var_decl.name = tokens->string;
            statements_it->var_decl.token_name = tokens;

            const gm_token_t *grave = tokens->next;
            if (grave->type != GM_TOKEN_TYPE_GRAVE)
            {
                gm_print_syntax_error_token(arena, grave, "expected grave/backtick");
                return false;
            }
            if (grave->next == nullptr)
            {
                gm_print_syntax_error_token(arena, grave, "unexpected end of file");
                return false;
            }

            const gm_token_t *type = grave->next;
            if (type->type != GM_TOKEN_TYPE_IDENTIFIER)
            {
                gm_print_syntax_error_token(arena, type, "expected identifier");
                return false;
            }
            if (strcmp(type->string.data, "s32") != 0)
            {
                gm_print_syntax_error_token(arena, type, "invalid type: \"%s\"", type->string.data);
                return false;
            }
            statements_it->var_decl.type = GM_TYPE_S32;
            if (type->next == nullptr)
            {
                gm_print_syntax_error_token(arena, type, "unexpected end of file");
                return false;
            }

            if (type->next->type == GM_TOKEN_TYPE_SEMICOLON)
            {
                statements_it->var_decl.expression = gm_expression_zero(arena);
                statements_it->var_decl.initialized = true;
                tokens = type->next->next;
                if (tokens == nullptr)
                {
                    gm_print_syntax_error_token(arena, type->next, "unexpected end of file");
                    return false;
                }
                continue;
            }

            const gm_token_t *equals = type->next;
            if (equals->type != GM_TOKEN_TYPE_EQUALS)
            {
                gm_print_syntax_error_token(arena, equals, "unexpected token");
                return false;
            }
            if (equals->next == nullptr)
            {
                gm_print_syntax_error_token(arena, equals, "unexpected end of file");
                return false;
            }

            if (equals->next->type == GM_TOKEN_TYPE_QUESTION)
            {
                const gm_token_t *question = equals->next;
                if (question->next == nullptr)
                {
                    gm_print_syntax_error_token(arena, question, "unexpected end of file");
                    return false;
                }
                const gm_token_t *semicolon = question->next;
                if (semicolon->type != GM_TOKEN_TYPE_SEMICOLON)
                {
                    gm_print_syntax_error_token(arena, semicolon, "expected semicolon");
                    return false;
                }
                statements_it->var_decl.initialized = false;
                tokens = semicolon->next;
                if (tokens == nullptr)
                {
                    gm_print_syntax_error_token(arena, semicolon, "unexpected end of file");
                    return false;
                }
                continue;
            }

            const gm_token_t *semicolon;
            if (!gm_parse_expression(arena, equals->next, &statements_it->var_decl.expression, &semicolon))
            {
                return false;
            }
            if (semicolon->type != GM_TOKEN_TYPE_SEMICOLON)
            {
                gm_print_syntax_error_token(arena, semicolon, "expected semicolon");
                return false;
            }
            statements_it->var_decl.initialized = true;
            tokens = semicolon->next;
            if (tokens == nullptr)
            {
                gm_print_syntax_error_token(arena, semicolon, "unexpected end of file");
                return false;
            }
        }
        else if (tokens->type == GM_TOKEN_TYPE_SEMICOLON)
        {
            gm_print_syntax_error_token(arena, tokens, "trailing semicolons not allowed");
            return false;
        }
        else
        {
            gm_print_syntax_error_token(arena, tokens, "unexpected token");
            return false;
        }
    }

    *statements = statements_result->next;
    *out = tokens->next;
    return true;
}

bool gm_ensure_fn_integrity(gm_arena_t *arena, const gm_token_t *fn_token, const gm_statement_t *statements)
{
    static_assert(GM_STATEMENT_TYPE_GUARDIAN_ANGEL == 2, "gm_print_statements(): statement type enum size changed");

    gm_arena_checkpoint_t mem_check = gm_arena_checkpoint_begin(arena);
    const char **var_names = gm_arena_alloc_array(arena, const char *, 1);
    size_t var_names_size = 1;

    bool returned = false;

    for (const gm_statement_t *it = statements; it != nullptr; it = it->next)
    {
        if (returned)
        {
            gm_print_syntax_error_token(arena, it->ret.token_return, "statement after the function has already returned");
            gm_arena_checkpoint_end(&mem_check);
            return false;
        }

        switch (it->type)
        {
        case GM_STATEMENT_TYPE_VAR_DECL:
            for (const char **i = var_names + 1; i < var_names + var_names_size; ++i)
            {
                if (strcmp(*i, it->var_decl.name.data) == 0)
                {
                    gm_print_syntax_error_token(arena, it->var_decl.token_name, "the name \"%s\" has already been used", it->var_decl.name.data);
                    gm_arena_checkpoint_end(&mem_check);
                    return false;
                }
            }
            var_names = gm_arena_realloc_array(arena, const char *, var_names, var_names_size, var_names_size + 1);
            var_names[var_names_size] = it->var_decl.name.data;
            ++var_names_size;
            break;
        case GM_STATEMENT_TYPE_RETURN:
            returned = true;
            break;
        default:
            fprintf(stderr, "\tERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
            gm_arena_checkpoint_end(&mem_check);
            return false;
        }
    }

    if (!returned)
    {
        gm_print_syntax_error_token(arena, fn_token, "the function \"%s\" has not returned", fn_token->string.data);
        gm_arena_checkpoint_end(&mem_check);
        return false;
    }

    gm_arena_checkpoint_end(&mem_check);
    return true;
}

void gm_print_statements(const gm_statement_t *statements)
{
    if (statements == nullptr)
    {
        puts("\t<NO STATEMENTS>");
        return;
    }

    static_assert(GM_STATEMENT_TYPE_GUARDIAN_ANGEL == 2, "gm_print_statements(): statement type enum size changed");
    for (const gm_statement_t *it = statements; it != nullptr; it = it->next)
    {
        switch (it->type)
        {
        case GM_STATEMENT_TYPE_VAR_DECL:
            printf("\tVARIABLE DECLARATION \"%s\" of type \"%s\" ", it->var_decl.name.data, gm_type_to_string(it->var_decl.type));
            if (it->var_decl.initialized)
            {
                puts("initialized with expression:");
                gm_print_expression(it->var_decl.expression, 10);
            }
            else
            {
                puts("uninitialized");
            }
            break;
        case GM_STATEMENT_TYPE_RETURN:
            puts("\tRETURN:");
            gm_print_expression(it->ret.expression, 10);
            break;
        default:
            fprintf(stderr, "\tERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
            return;
        }
    }
}

#include <gamma/parser/expression.h>
#include <gamma/parser/statement.h>
#include <gamma/parser/type.h>
#include <gamma/utils/print.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

bool gm_parse_fn_body(gm_arena_t *arena, const gm_token_t *tokens, gm_statement_t **statements, const gm_token_t **out)
{
    static_assert(GM_TOKEN_TYPE_GUARDIAN_ANGEL == 19, "gm_parse_fn_body(): token type enum size changed");
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

    gm_fn_parse_ctx_t ctx = gm_fn_parse_ctx_new(arena);

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

            if (!gm_parse_expression(arena, tokens->next, &ctx, &statements_it->ret.expression, &tokens))
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
            const gm_statement_t *found = gm_fn_parse_ctx_find_var_decl(&ctx, tokens->string.data);

            if (found)
            {
                gm_print_syntax_error_token(arena, tokens, "variable with this name has already been declared");
                return false;
            }

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

            const gm_token_t *out;
            if (!gm_parse_type(arena, grave->next, &statements_it->var_decl.type, &out))
            {
                return false;
            }

            if (out->type == GM_TOKEN_TYPE_SEMICOLON)
            {
                statements_it->var_decl.expression = gm_expression_zero(arena);
                statements_it->var_decl.initialized = true;
                tokens = out->next;
                if (tokens == nullptr)
                {
                    gm_print_syntax_error_token(arena, out, "unexpected end of file");
                    return false;
                }
                continue;
            }

            const gm_token_t *equals = out;
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
            if (!gm_parse_expression(arena, equals->next, &ctx, &statements_it->var_decl.expression, &semicolon))
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

            gm_fn_parse_ctx_add_var_decl(arena, &ctx, statements_it);
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

    bool returned = false;

    for (const gm_statement_t *it = statements; it != nullptr; it = it->next)
    {
        if (returned)
        {
            gm_print_syntax_error_token(arena, it->ret.token_return, "statement after the function has already returned");
            return false;
        }

        switch (it->type)
        {
        case GM_STATEMENT_TYPE_VAR_DECL:
            if (!it->var_decl.used)
            {
                gm_print_syntax_error_token(arena, it->var_decl.token_name, "variable declared, but not used");
                return false;
            }
            break;
        case GM_STATEMENT_TYPE_RETURN:
            returned = true;
            break;
        default:
            fprintf(stderr, "\tERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
            return false;
        }
    }

    if (!returned)
    {
        gm_print_syntax_error_token(arena, fn_token, "the function \"%s\" has not returned", fn_token->string.data);
        return false;
    }

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

gm_fn_parse_ctx_t gm_fn_parse_ctx_new(gm_arena_t *arena)
{
    return (gm_fn_parse_ctx_t) {
        .var_decls = gm_arena_alloc_array(arena, gm_statement_t *, 16),
        .var_decls_size = 0,
        .var_decls_cap = 16
    };
}

void gm_fn_parse_ctx_add_var_decl(gm_arena_t *arena, gm_fn_parse_ctx_t *ctx, gm_statement_t *decl)
{
    if (ctx->var_decls_size >= ctx->var_decls_cap)
    {
        ctx->var_decls = gm_arena_realloc_array(arena, gm_statement_t *, ctx->var_decls, ctx->var_decls_cap, ctx->var_decls_cap * 3);
        ctx->var_decls_cap *= 3;
    }
    ctx->var_decls[ctx->var_decls_size++] = decl;
}

gm_statement_t *gm_fn_parse_ctx_find_var_decl(gm_fn_parse_ctx_t *ctx, const char *var_name)
{
    for (gm_statement_t **i = ctx->var_decls; i < ctx->var_decls + ctx->var_decls_size; ++i)
    {
        if (strcmp(var_name, (*i)->var_decl.name.data) == 0)
        {
            return *i;
        }
    }
    return nullptr;
}

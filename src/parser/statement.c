#include <gamma/parser/statement.h>
#include <gamma/utils/print.h>
#include <inttypes.h>
#include <stdio.h>

bool gm_parse_fn_body(gm_arena_t *arena, const gm_token_t *tokens, gm_statement_t **statements, const gm_token_t **out)
{
    static_assert(GM_TOKEN_TYPE_GUARDIAN_ANGEL == 12, "gm_parse_fn_body(): token type enum size changed");
    static_assert(GM_STATEMENT_TYPE_GUARDIAN_ANGEL == 1, "gm_parse_fn_body(): statement type enum size changed");

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
        if (tokens->type != GM_TOKEN_TYPE_RETURN)
        {
            gm_print_syntax_error_token(arena, tokens, "expected return keyword");
            return false;
        }
        statements_it->next = gm_arena_alloc_single(arena, gm_statement_t);
        statements_it = statements_it->next;
        statements_it->type = GM_STATEMENT_TYPE_RETURN;
        statements_it->token_return = tokens;

        const gm_token_t *number = tokens->next;
        if (number->type != GM_TOKEN_TYPE_NUMBER)
        {
            gm_print_syntax_error_token(arena, number, "expected a number");
            return false;
        }
        statements_it->number = number->number;
        statements_it->token_number = number;

        if (number->next == nullptr)
        {
            gm_print_syntax_error_token(arena, number, "unexpected end of file");
            return false;
        }

        const gm_token_t *semicolon = number->next;
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

    *statements = statements_result->next;
    *out = tokens->next;
    return true;
}

bool gm_ensure_fn_integrity(gm_arena_t *arena, const gm_token_t *fn_token, const gm_statement_t *statements)
{
    static_assert(GM_STATEMENT_TYPE_GUARDIAN_ANGEL == 1, "gm_print_statements(): statement type enum size changed");

    bool returned = false;

    for (const gm_statement_t *it = statements; it != nullptr; it = it->next)
    {
        if (returned)
        {
            gm_print_syntax_error_token(arena, it->token_return, "statement after the function has already returned");
            return false;
        }

        switch (it->type)
        {
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

    static_assert(GM_STATEMENT_TYPE_GUARDIAN_ANGEL == 1, "gm_print_statements(): statement type enum size changed");
    for (const gm_statement_t *it = statements; it != nullptr; it = it->next)
    {
        switch (it->type)
        {
        case GM_STATEMENT_TYPE_RETURN:
            printf("\tRETURN %" PRId32 "\n", it->number);
            break;
        default:
            fprintf(stderr, "\tERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
            return;
        }
    }
}

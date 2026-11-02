#include <gamma/lexer/lexer.h>
#include <gamma/parser/type.h>
#include <gamma/utils/print.h>
#include <stdio.h>
#include <string.h>

bool gm_parse_type(gm_arena_t *arena, const gm_token_t *tokens, gm_type_t *type, const gm_token_t **out)
{
    static_assert(GM_TOKEN_TYPE_GUARDIAN_ANGEL == 18, "gm_parse_type(): token type enum size changed");
    static_assert(GM_TYPE_KIND_GUARDIAN_ANGEL == 1, "gm_parse_type(): type kind enum size changed");

    if (tokens->type != GM_TOKEN_TYPE_IDENTIFIER)
    {
        gm_print_syntax_error_token(arena, tokens, "unexpected token");
        return false;
    }

    if (strcmp(tokens->string.data, "s32") != 0)
    {
        gm_print_syntax_error_token(arena, tokens, "undeclared identifier");
        return false;
    }

    if (tokens->next == nullptr)
    {
        gm_print_syntax_error_token(arena, tokens, "unexpected end of file");
        return false;
    }

    type->kind = GM_TYPE_KIND_S32;
    *out = tokens->next;
    return true;
}

const char *gm_type_to_string(const gm_type_t type)
{
    static_assert(GM_TYPE_KIND_GUARDIAN_ANGEL == 1, "gm_type_to_string(): type kind enum size changed");

    switch (type.kind)
    {
    case GM_TYPE_KIND_S32:
        return "s32";
    default:
        fprintf(stderr, "ERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
        return "<UNREACHABLE>";
    }
}

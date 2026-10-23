#ifndef GM_API_LEXER_LEXER_H
#define GM_API_LEXER_LEXER_H

#include <gamma/arena/arena.h>
#include <gamma/string/string.h>
#include <stddef.h>
#include <stdint.h>

typedef enum gm_token_type
{
    GM_TOKEN_TYPE_IDENTIFIER,
    GM_TOKEN_TYPE_NUMBER,

    GM_TOKEN_TYPE_FN,
    GM_TOKEN_TYPE_RETURN,

    GM_TOKEN_TYPE_LEFT_PAREN,
    GM_TOKEN_TYPE_RIGHT_PAREN,
    GM_TOKEN_TYPE_LEFT_CURLY,
    GM_TOKEN_TYPE_RIGHT_CURLY,
    GM_TOKEN_TYPE_LEFT_SQUARE,
    GM_TOKEN_TYPE_RIGHT_SQUARE,
    GM_TOKEN_TYPE_SEMICOLON,

    GM_TOKEN_TYPE_GUARDIAN_ANGEL
} gm_token_type_e;

typedef struct gm_token gm_token_t;
struct gm_token
{
    gm_token_t *next;

    gm_token_type_e type;
    size_t line;
    size_t column;

    union
    {
        // GM_LEX_TYPE_IDENTIFIER
        gm_string_t string;

        // GM_LEX_TYPE_NUMBER
        int32_t number;
    };
};

bool gm_lex_file(gm_arena_t *arena, const char *path, const gm_token_t **out);

void gm_print_tokens(const gm_token_t *tokens);

#endif // GM_API_LEXER_LEXER_H

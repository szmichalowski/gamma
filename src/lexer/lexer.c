#include <ctype.h>
#include <gamma/lexer/lexer.h>
#include <gamma/utils/print.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

bool gm_lex_file(gm_arena_t *arena, const char *path, const char *code, const gm_token_t **out)
{
    static_assert(GM_TOKEN_TYPE_GUARDIAN_ANGEL == 18, "gm_lex_file(): token type enum size changed");

    gm_token_t *result = gm_arena_alloc_single(arena, gm_token_t);
    gm_token_t *it = result;

    const char *ch = code;
    const char *line_start = code;
    size_t line = 1;
    size_t column = 1;
    while (*ch != 0)
    {
        if (*ch == '\n')
        {
            ++line;
            line_start = ch + 1;
            column = 1;
            ++ch;
            continue;
        }

        if (isspace(*ch))
        {
            ++column;
            ++ch;
            continue;
        }

        if (*ch == '+')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_PLUS,
                .line = line,
                .column = column,
                .width = 1,
                .source_file = path,
                .line_start = line_start
            };
            ++column;
            ++ch;
            continue;
        }

        if (*ch == '-')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_MINUS,
                .line = line,
                .column = column,
                .width = 1,
                .source_file = path,
                .line_start = line_start
            };
            ++column;
            ++ch;
            continue;
        }

        if (*ch == '*')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_STAR,
                .line = line,
                .column = column,
                .width = 1,
                .source_file = path,
                .line_start = line_start
            };
            ++column;
            ++ch;
            continue;
        }

        if (*ch == '/')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_SLASH,
                .line = line,
                .column = column,
                .width = 1,
                .source_file = path,
                .line_start = line_start
            };
            ++column;
            ++ch;
            continue;
        }

        if (*ch == '=')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_EQUALS,
                .line = line,
                .column = column,
                .width = 1,
                .source_file = path,
                .line_start = line_start
            };
            ++column;
            ++ch;
            continue;
        }

        if (*ch == '?')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_QUESTION,
                .line = line,
                .column = column,
                .width = 1,
                .source_file = path,
                .line_start = line_start
            };
            ++column;
            ++ch;
            continue;
        }

        if (*ch == '`')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_GRAVE,
                .line = line,
                .column = column,
                .width = 1,
                .source_file = path,
                .line_start = line_start
            };
            ++column;
            ++ch;
            continue;
        }

        if (*ch == '(')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_LEFT_PAREN,
                .line = line,
                .column = column,
                .width = 1,
                .source_file = path,
                .line_start = line_start
            };
            ++column;
            ++ch;
            continue;
        }

        if (*ch == ')')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_RIGHT_PAREN,
                .line = line,
                .column = column,
                .width = 1,
                .source_file = path,
                .line_start = line_start
            };
            ++column;
            ++ch;
            continue;
        }

        if (*ch == '{')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_LEFT_CURLY,
                .line = line,
                .column = column,
                .width = 1,
                .source_file = path,
                .line_start = line_start
            };
            ++column;
            ++ch;
            continue;
        }

        if (*ch == '}')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_RIGHT_CURLY,
                .line = line,
                .column = column,
                .width = 1,
                .source_file = path,
                .line_start = line_start
            };
            ++column;
            ++ch;
            continue;
        }

        if (*ch == '[')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_LEFT_SQUARE,
                .line = line,
                .column = column,
                .width = 1,
                .source_file = path,
                .line_start = line_start
            };
            ++column;
            ++ch;
            continue;
        }

        if (*ch == ']')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_RIGHT_SQUARE,
                .line = line,
                .column = column,
                .width = 1,
                .source_file = path,
                .line_start = line_start
            };
            ++column;
            ++ch;
            continue;
        }

        if (*ch == ';')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_SEMICOLON,
                .line = line,
                .column = column,
                .width = 1,
                .source_file = path,
                .line_start = line_start
            };
            ++column;
            ++ch;
            continue;
        }

        if (!isalnum(*ch) && *ch != '_')
        {
            gm_print_syntax_error(arena, line, column, 1, path, line_start, "character \"%c\" (%#x) is not allowed", *ch, *ch);
            return false;
        }

        if (isdigit(*ch))
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_NUMBER,
                .line = line,
                .column = column,
                .width = 1,
                .source_file = path,
                .line_start = line_start,
                .number = *ch - '0'
            };
            ++ch;

            while (*ch != 0)
            {
                if (isdigit(*ch))
                {
                    it->number *= 10;
                    it->number += *ch - '0';
                    ++it->width;
                    ++column;
                    ++ch;
                    continue;
                }

                if (isalpha(*ch))
                {
                    gm_print_syntax_error(arena, line, column + 1, 1, path, line_start, "character \"%c\" after a digit is not allowed", *ch);
                    return false;
                }

                break;
            }

            ++column;
            continue;
        }

        if (isalpha(*ch) || *ch == '_')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_IDENTIFIER,
                .line = line,
                .column = column,
                .width = 1,
                .source_file = path,
                .line_start = line_start
            };
            gm_string_init(arena, &it->string);
            gm_string_append_char(arena, &it->string, *ch);
            ++ch;

            while (*ch != 0)
            {
                if (isalpha(*ch) || isdigit(*ch) || *ch == '_')
                {
                    gm_string_append_char(arena, &it->string, *ch);
                    ++it->width;
                    ++column;
                    ++ch;
                    continue;
                }

                break;
            }

            if (strcmp(it->string.data, "fn") == 0)
            {
                it->type = GM_TOKEN_TYPE_FN;
            }
            else if (strcmp(it->string.data, "return") == 0)
            {
                it->type = GM_TOKEN_TYPE_RETURN;
            }

            ++column;
            continue;
        }

        fprintf(stderr, "ERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
        return false;
    }

    *out = result->next;
    return true;
}

void gm_print_tokens(const gm_token_t *tokens)
{
    if (tokens == nullptr)
    {
        puts("<NO TOKENS>");
        return;
    }

    static_assert(GM_TOKEN_TYPE_GUARDIAN_ANGEL == 18, "gm_lex_print(): token type enum size changed");
    for (const gm_token_t *it = tokens; it != nullptr; it = it->next)
    {
        printf("%zu:%zu: ", it->line, it->column);

        switch (it->type)
        {
        case GM_TOKEN_TYPE_IDENTIFIER:
            printf("IDENTIFIER: \"%s\"\n", it->string.data);
            break;
        case GM_TOKEN_TYPE_NUMBER:
            printf("NUMBER: %" PRId32 "\n", it->number);
            break;
        case GM_TOKEN_TYPE_FN:
            puts("FN");
            break;
        case GM_TOKEN_TYPE_RETURN:
            puts("RETURN");
            break;
        case GM_TOKEN_TYPE_PLUS:
            puts("PLUS");
            break;
        case GM_TOKEN_TYPE_MINUS:
            puts("MINUS");
            break;
        case GM_TOKEN_TYPE_STAR:
            puts("STAR");
            break;
        case GM_TOKEN_TYPE_SLASH:
            puts("SLASH");
            break;
        case GM_TOKEN_TYPE_EQUALS:
            puts("EQUALS SIGN");
            break;
        case GM_TOKEN_TYPE_QUESTION:
            puts("QUESTION MARK");
            break;
        case GM_TOKEN_TYPE_GRAVE:
            puts("GRAVE/BACKTICK");
            break;
        case GM_TOKEN_TYPE_LEFT_PAREN:
            puts("LEFT PARENTHESIS");
            break;
        case GM_TOKEN_TYPE_RIGHT_PAREN:
            puts("RIGHT PARENTHESIS");
            break;
        case GM_TOKEN_TYPE_LEFT_CURLY:
            puts("LEFT CURLY BRACKET");
            break;
        case GM_TOKEN_TYPE_RIGHT_CURLY:
            puts("RIGHT CURLY BRACKET");
            break;
        case GM_TOKEN_TYPE_LEFT_SQUARE:
            puts("LEFT SQUARE BRACKET");
            break;
        case GM_TOKEN_TYPE_RIGHT_SQUARE:
            puts("RIGHT SQUARE BRACKET");
            break;
        case GM_TOKEN_TYPE_SEMICOLON:
            puts("SEMICOLON");
            break;
        default:
            fprintf(stderr, "ERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
            return;
        }
    }
}

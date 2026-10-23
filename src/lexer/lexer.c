#include <ctype.h>
#include <gamma/lexer/lexer.h>
#include <gamma/utils/print.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

bool gm_lex_file(gm_arena_t *arena, const char *path, const gm_token_t **out)
{
    static_assert(GM_TOKEN_TYPE_GUARDIAN_ANGEL == 11, "gm_lex_file(): token type enum size changed");

    FILE *file = fopen(path, "r");
    if (file == nullptr)
    {
        perror("ERROR: gm_lex_file(): fopen()");
        return false;
    }

    gm_token_t *result = gm_arena_alloc_single(arena, gm_token_t);
    gm_token_t *it = result;
    if (result == nullptr)
    {
        fputs("ERROR: gm_lex_file(): allocation error\n", stderr);
        fclose(file);
        return false;
    }

    int ch;
    size_t line = 1;
    size_t column = 1;
    while ((ch = fgetc(file)) != EOF)
    {
        if (ch == '\n')
        {
            ++line;
            column = 1;
            continue;
        }

        if (isspace(ch))
        {
            ++column;
            continue;
        }

        if (ch == '(')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            if (it->next == nullptr)
            {
                fputs("ERROR: gm_lex_file(): allocation error\n", stderr);
                fclose(file);
                return false;
            }

            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_LEFT_PAREN,
                .line = line,
                .column = column
            };
            ++column;
            continue;
        }

        if (ch == ')')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            if (it->next == nullptr)
            {
                fputs("ERROR: gm_lex_file(): allocation error\n", stderr);
                fclose(file);
                return false;
            }

            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_RIGHT_PAREN,
                .line = line,
                .column = column
            };
            ++column;
            continue;
        }

        if (ch == '{')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            if (it->next == nullptr)
            {
                fputs("ERROR: gm_lex_file(): allocation error\n", stderr);
                fclose(file);
                return false;
            }

            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_LEFT_CURLY,
                .line = line,
                .column = column
            };
            ++column;
            continue;
        }

        if (ch == '}')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            if (it->next == nullptr)
            {
                fputs("ERROR: gm_lex_file(): allocation error\n", stderr);
                fclose(file);
                return false;
            }

            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_RIGHT_CURLY,
                .line = line,
                .column = column
            };
            ++column;
            continue;
        }

        if (ch == '[')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            if (it->next == nullptr)
            {
                fputs("ERROR: gm_lex_file(): allocation error\n", stderr);
                fclose(file);
                return false;
            }

            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_LEFT_SQUARE,
                .line = line,
                .column = column
            };
            ++column;
            continue;
        }

        if (ch == ']')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            if (it->next == nullptr)
            {
                fputs("ERROR: gm_lex_file(): allocation error\n", stderr);
                fclose(file);
                return false;
            }

            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_RIGHT_SQUARE,
                .line = line,
                .column = column
            };
            ++column;
            continue;
        }

        if (ch == ';')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            if (it->next == nullptr)
            {
                fputs("ERROR: gm_lex_file(): allocation error\n", stderr);
                fclose(file);
                return false;
            }

            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_SEMICOLON,
                .line = line,
                .column = column
            };
            ++column;
            continue;
        }

        if (!isalnum(ch) && ch != '_')
        {
            gm_print_syntax_error(arena, line, column, "character \"%c\" (%#x) is not allowed", ch, ch);
            return false;
        }

        if (isdigit(ch))
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            if (it->next == nullptr)
            {
                fputs("ERROR: gm_lex_file(): allocation error\n", stderr);
                fclose(file);
                return false;
            }

            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_NUMBER,
                .line = line,
                .column = column,
                .number = ch - '0'
            };

            while ((ch = fgetc(file)) != EOF)
            {
                if (isdigit(ch))
                {
                    it->number *= 10;
                    it->number += ch - '0';
                    ++column;
                    continue;
                }

                if (isalpha(ch))
                {
                    gm_print_syntax_error(arena, line, column + 1, "character \"%c\" after a digit is not allowed", ch);
                    return false;
                }

                break;
            }

            if (ungetc(ch, file) == EOF)
            {
                perror("ERROR: gm_lex_file(): ungetc()");
                return false;
            }

            ++column;
            continue;
        }

        if (isalpha(ch) || ch == '_')
        {
            it->next = gm_arena_alloc_single(arena, gm_token_t);
            if (it->next == nullptr)
            {
                fputs("ERROR: gm_lex_file(): allocation error\n", stderr);
                fclose(file);
                return false;
            }

            it = it->next;
            *it = (gm_token_t) {
                .type = GM_TOKEN_TYPE_IDENTIFIER,
                .line = line,
                .column = column
            };
            if (!gm_string_init(arena, &it->string))
            {
                fputs("ERROR: gm_lex_file(): allocation error\n", stderr);
                fclose(file);
                return false;
            }
            if (!gm_string_append_char(arena, &it->string, ch))
            {
                fputs("ERROR: gm_lex_file(): allocation error\n", stderr);
                fclose(file);
                return false;
            }

            while ((ch = fgetc(file)) != EOF)
            {
                if (isalpha(ch) || isdigit(ch) || ch == '_')
                {
                    if (!gm_string_append_char(arena, &it->string, ch))
                    {
                        fputs("ERROR: gm_lex_file(): allocation error\n", stderr);
                        fclose(file);
                        return false;
                    }
                    ++column;
                    continue;
                }

                break;
            }

            if (ungetc(ch, file) == EOF)
            {
                perror("ERROR: gm_lex_file(): ungetc()");
                return false;
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

    if (fclose(file) != 0)
    {
        perror("ERROR: gm_lex_file(): fclose()");
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

    static_assert(GM_TOKEN_TYPE_GUARDIAN_ANGEL == 11, "gm_lex_print(): token type enum size changed");
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

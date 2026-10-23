#include <gamma/arena/arena.h>
#include <gamma/lexer/lexer.h>
#include <gamma/parser/parser.h>
#include <stdio.h>
#include <stdlib.h>

int main(const int argc, const char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <source_file.gm>\n", argv[0]);
        return EXIT_FAILURE;
    }

    gm_arena_t arena;
    gm_arena_init(&arena, 0);

    const gm_token_t *tokens;
    if (!gm_lex_file(&arena, argv[1], &tokens))
    {
        gm_arena_destroy(&arena);
        return EXIT_FAILURE;
    }

    puts("===== Tokens =====");
    gm_print_tokens(tokens);
    putchar('\n');
    putchar('\n');

    const gm_ast_node_t *ast;
    if (!gm_parse_tokens(&arena, tokens, &ast))
    {
        gm_arena_destroy(&arena);
        return EXIT_FAILURE;
    }

    puts("===== Abstract Syntax Tree =====");
    gm_print_ast_tree(ast);

    if (!gm_ensure_ast_integrity(&arena, ast))
    {
        gm_arena_destroy(&arena);
        return EXIT_FAILURE;
    }

    gm_arena_destroy(&arena);

    return EXIT_SUCCESS;
}

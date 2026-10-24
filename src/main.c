#include <gamma/arena/arena.h>
#include <gamma/generator/generator.h>
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
        goto failure;
    }

    puts("===== Tokens =====");
    gm_print_tokens(tokens);
    putchar('\n');
    putchar('\n');

    gm_ast_node_t *ast;
    if (!gm_parse_tokens(&arena, tokens, &ast))
    {
        goto failure;
    }

    puts("===== Abstract Syntax Tree =====");
    gm_print_ast_tree(ast);
    putchar('\n');
    putchar('\n');

    if (!gm_ensure_ast_integrity(&arena, ast))
    {
        goto failure;
    }

    if (!gm_amd64_compile_ast_tree(&arena, ast))
    {
        goto failure;
    }

    puts("===== AMD64 Compiled Abstract Syntax Tree =====");
    gm_amd64_print_ast_tree(ast);
    putchar('\n');
    putchar('\n');

    gm_arena_destroy(&arena);
    return EXIT_SUCCESS;

failure:
    gm_arena_destroy(&arena);
    return EXIT_FAILURE;
}

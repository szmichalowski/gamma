#include <gamma/arena/arena.h>
#include <gamma/generator/elf.h>
#include <gamma/generator/generator.h>
#include <gamma/lexer/lexer.h>
#include <gamma/parser/parser.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

static constexpr int alloc_failure_code = 17;

int main(const int argc, const char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <source_file.gm>\n", argv[0]);
        return EXIT_FAILURE;
    }

    gm_arena_t arena;
    gm_arena_init(&arena, 0);

    jmp_buf longjump_buffer;
    gm_arena_set_callback(&arena, &longjump_buffer, alloc_failure_code);

    const int jmp_ret = setjmp(longjump_buffer);
    if (jmp_ret != 0)
    {
        if (jmp_ret == alloc_failure_code)
        {
            fputs("main(): caught allocation error code, exitting...\n", stderr);
        }
        else
        {
            fputs("main(): caught unknown error code, exitting...\n", stderr);
        }
        goto failure;
    }

    FILE *src_file = fopen(argv[1], "r");
    if (src_file == nullptr)
    {
        perror("ERROR: main(): fopen()");
        goto failure;
    }

    int fd = fileno(src_file);
    if (fd == -1)
    {
        perror("ERROR: main(): fileno()");
        fclose(src_file);
        goto failure;
    }

    struct stat st;
    if (fstat(fd, &st) == -1)
    {
        perror("ERROR: main(): fstat()");
        fclose(src_file);
        goto failure;
    }

    char *source_code = gm_arena_alloc_array(&arena, char, st.st_size + 1);
    char *source_code_it = source_code;
    size_t rem = st.st_size;
    do
    {
        const size_t change = fread(source_code_it, 1, rem, src_file);
        if (change == 0)
        {
            perror("ERROR: main(): fread()");
            fclose(src_file);
            goto failure;
        }
        source_code_it += change;
        rem -= change;
    } while (rem > 0);

    if (fclose(src_file) == EOF)
    {
        perror("ERROR: main(): fclose()");
        goto failure;
    }

    const gm_token_t *tokens;
    if (!gm_lex_file(&arena, argv[1], source_code, &tokens))
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

    if (!gm_amd64_virtual_compile_ast_tree(&arena, ast))
    {
        goto failure;
    }

    puts("===== AMD64 Compiled Abstract Syntax Tree =====");
    gm_amd64_print_ast_tree(ast);
    putchar('\n');
    putchar('\n');

    if (!gm_amd64_physical_compile_ast_tree(&arena, ast))
    {
        goto failure;
    }

    const uint8_t *image_data;
    size_t image_size;
    if (!gm_amd64_generate_executable_image(&arena, ast, &image_data, &image_size))
    {
        goto failure;
    }
    printf("----- Generated image size = %zu bytes -----\n\n", image_size);

    FILE *out_file = fopen("wynik.out", "w");
    if (out_file == nullptr)
    {
        perror("ERROR: main(): fopen()");
        goto failure;
    }

    if (fwrite(image_data, 1, image_size, out_file) != image_size)
    {
        perror("ERROR: main(): fwrite()");
        fclose(out_file);
        goto failure;
    }

    fd = fileno(out_file);
    if (fd == -1)
    {
        perror("ERROR: main(): fileno()");
        fclose(out_file);
        goto failure;
    }

    if (fchmod(fd, 0775) == -1)
    {
        perror("ERROR: main(): fchmod()");
        fclose(out_file);
        goto failure;
    }

    if (fclose(out_file) != 0)
    {
        perror("ERROR: main(): fclose()");
        goto failure;
    }

    printf("----- Generated image file = \"wynik.out\" -----\n\n");

    gm_arena_destroy(&arena);
    return EXIT_SUCCESS;

failure:
    gm_arena_destroy(&arena);
    return EXIT_FAILURE;
}

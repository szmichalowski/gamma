#ifndef GM_API_GENERATOR_GENERATOR_H
#define GM_API_GENERATOR_GENERATOR_H

#include <gamma/arena/arena.h>
#include <gamma/parser/parser.h>
#include <stdint.h>

typedef enum gm_amd64_instruction_type
{
    GM_AMD64_INSTRUCTION_TYPE_MOV_REG_NUM,

    GM_AMD64_INSTRUCTION_TYPE_RET,
    GM_AMD64_INSTRUCTION_TYPE_SYSCALL,

    GM_AMD64_INSTRUCTION_TYPE_GUARDIAN_ANGEL
} gm_amd64_instruction_type_e;

typedef enum gm_amd64_register_type
{
    GM_AMD64_REGISTER_TYPE_EAX,
    GM_AMD64_REGISTER_TYPE_EBX,
    GM_AMD64_REGISTER_TYPE_ECX,
    GM_AMD64_REGISTER_TYPE_EDX,
    GM_AMD64_REGISTER_TYPE_EDI,
    GM_AMD64_REGISTER_TYPE_ESI,

    GM_AMD64_REGISTER_TYPE_GUARDIAN_ANGEL
} gm_amd64_register_type_e;

typedef struct gm_amd64_instruction gm_amd64_instruction_t;
struct gm_amd64_instruction
{
    gm_amd64_instruction_t *next;

    gm_amd64_instruction_type_e type;
    union
    {
        // GM_AMD64_INSTRUCTION_TYPE_MOV_REG_NUM
        struct
        {
            gm_amd64_register_type_e register_type;
            int32_t number;
        };
    };
};

bool gm_amd64_virtual_compile_ast_tree(gm_arena_t *arena, gm_ast_node_t *ast_tree);

bool gm_amd64_physical_compile_ast_tree(gm_arena_t *arena, gm_ast_node_t *ast_tree);

void gm_amd64_print_ast_tree(const gm_ast_node_t *ast_tree);

#endif // GM_API_GENERATOR_GENERATOR_H

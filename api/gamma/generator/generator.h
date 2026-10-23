#ifndef GM_API_GENERATOR_GENERATOR_H
#define GM_API_GENERATOR_GENERATOR_H

#include <gamma/arena/arena.h>
#include <gamma/parser/statement.h>
#include <stdint.h>

typedef enum gm_amd64_instruction_type
{
    GM_AMD64_INSTRUCTION_TYPE_MOV_REG_NUM,

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

bool gm_amd64_generate_fn_body(gm_arena_t *arena, const gm_statement_t *statements, gm_amd64_instruction_t **instructions);

void gm_print_amd64_instructions(const gm_amd64_instruction_t *instructions);

#endif // GM_API_GENERATOR_GENERATOR_H

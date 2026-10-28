#include "gamma/parser/statement.h"
#include <gamma/generator/generator.h>
#include <inttypes.h>
#include <stdio.h>

static bool gm_amd64_generate_fn_body(gm_arena_t *arena, const gm_statement_t *statements, gm_amd64_instruction_t **instructions)
{
    static_assert(GM_AMD64_INSTRUCTION_TYPE_GUARDIAN_ANGEL == 3, "gm_amd64_generate_fn_body(): amd64 instruction type enum size changed");
    static_assert(GM_AMD64_REGISTER_TYPE_GUARDIAN_ANGEL == 6, "gm_amd64_generate_fn_body(): amd64 register type enum size changed");
    static_assert(GM_STATEMENT_TYPE_GUARDIAN_ANGEL == 2, "gm_amd64_generate_fn_body(): statement type enum size changed");

    gm_amd64_instruction_t *instructions_result = gm_arena_alloc_single(arena, gm_amd64_instruction_t);
    gm_amd64_instruction_t *instructions_it = instructions_result;

    for (const gm_statement_t *stmt = statements; stmt != nullptr; stmt = stmt->next)
    {
        switch (stmt->type)
        {
        case GM_STATEMENT_TYPE_VAR_DECL:
            break;
        case GM_STATEMENT_TYPE_RETURN:
            instructions_it->next = gm_arena_alloc_single(arena, gm_amd64_instruction_t);
            instructions_it = instructions_it->next;
            instructions_it->type = GM_AMD64_INSTRUCTION_TYPE_MOV_REG_NUM;
            instructions_it->register_type = GM_AMD64_REGISTER_TYPE_EAX;
            instructions_it->number = stmt->ret.number;

            instructions_it->next = gm_arena_alloc_single(arena, gm_amd64_instruction_t);
            instructions_it = instructions_it->next;
            instructions_it->type = GM_AMD64_INSTRUCTION_TYPE_RET;

            break;
        default:
            fprintf(stderr, "ERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
            return false;
        }
    }

    *instructions = instructions_result->next;
    return true;
}

bool gm_amd64_virtual_compile_ast_tree(gm_arena_t *arena, gm_ast_node_t *ast_tree)
{
    static_assert(GM_AST_NODE_TYPE_GUARDIAN_ANGEL == 1, "gm_amd64_virtual_compile_ast_tree(): AST node type enum size changed");
    for (gm_ast_node_t *it = ast_tree; it != nullptr; it = it->next)
    {
        switch (it->type)
        {
        case GM_AST_NODE_TYPE_FN:
            if (!gm_amd64_generate_fn_body(arena, it->fn.statements, &it->fn.instructions))
            {
                return false;
            }

            break;
        default:
            fprintf(stderr, "ERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
            return false;
        }
    }

    return true;
}

static uint8_t gm_amd64_register_to_number(const gm_amd64_register_type_e register_type)
{
    static_assert(GM_AMD64_REGISTER_TYPE_GUARDIAN_ANGEL == 6, "gm_amd64_register_to_string(): AMD64 register type enum size changed");

    switch (register_type)
    {
    case GM_AMD64_REGISTER_TYPE_EAX:
        return 0;
    case GM_AMD64_REGISTER_TYPE_EBX:
        return 3;
    case GM_AMD64_REGISTER_TYPE_ECX:
        return 1;
    case GM_AMD64_REGISTER_TYPE_EDX:
        return 2;
    case GM_AMD64_REGISTER_TYPE_EDI:
        return 7;
    case GM_AMD64_REGISTER_TYPE_ESI:
        return 6;
    default:
        fprintf(stderr, "\tERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
        return 0;
    }
}

static bool gm_amd64_generate_fn_binary(gm_arena_t *arena, const gm_amd64_instruction_t *instructions, uint8_t **binary_data, size_t *binary_data_size)
{
    static_assert(GM_AMD64_INSTRUCTION_TYPE_GUARDIAN_ANGEL == 3, "gm_amd64_generate_fn_binary(): amd64 instruction type enum size changed");
    static_assert(GM_AMD64_REGISTER_TYPE_GUARDIAN_ANGEL == 6, "gm_amd64_generate_fn_binary(): amd64 register type enum size changed");

    uint8_t *binary_data_result = gm_arena_alloc_array(arena, uint8_t, 1);
    uint8_t *binary_data_it = binary_data_result;
    size_t size = 1;

    for (const gm_amd64_instruction_t *inst = instructions; inst != nullptr; inst = inst->next)
    {
        switch (inst->type)
        {
        case GM_AMD64_INSTRUCTION_TYPE_MOV_REG_NUM:
            binary_data_result = gm_arena_realloc_array(arena, uint8_t, binary_data_result, size, size + 5);
            binary_data_it = binary_data_result + size - 1;
            binary_data_it[1] = 0xB8 + gm_amd64_register_to_number(inst->register_type);
            binary_data_it[2] = inst->number & 0xFF;
            binary_data_it[3] = (inst->number >> 8) & 0xFF;
            binary_data_it[4] = (inst->number >> 16) & 0xFF;
            binary_data_it[5] = (inst->number >> 24) & 0xFF;
            binary_data_it += 5;
            size += 5;
            break;
        case GM_AMD64_INSTRUCTION_TYPE_RET:
            binary_data_result = gm_arena_realloc_array(arena, uint8_t, binary_data_result, size, size + 1);
            binary_data_it = binary_data_result + size - 1;
            binary_data_it[1] = 0xC3;
            binary_data_it += 1;
            size += 1;
            break;
        case GM_AMD64_INSTRUCTION_TYPE_SYSCALL:
            binary_data_result = gm_arena_realloc_array(arena, uint8_t, binary_data_result, size, size + 2);
            binary_data_it = binary_data_result + size - 1;
            binary_data_it[1] = 0x0F;
            binary_data_it[2] = 0x05;
            binary_data_it += 2;
            size += 2;
            break;
        default:
            fprintf(stderr, "ERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
            return false;
        }
    }

    *binary_data = size > 1 ? binary_data_result + 1 : nullptr;
    *binary_data_size = size - 1;
    return true;
}

bool gm_amd64_physical_compile_ast_tree(gm_arena_t *arena, gm_ast_node_t *ast_tree)
{
    static_assert(GM_AST_NODE_TYPE_GUARDIAN_ANGEL == 1, "gm_amd64_physical_compile_ast_tree(): AST node type enum size changed");
    for (gm_ast_node_t *it = ast_tree; it != nullptr; it = it->next)
    {
        switch (it->type)
        {
        case GM_AST_NODE_TYPE_FN:
            if (!gm_amd64_generate_fn_binary(arena, it->fn.instructions, &it->fn.binary_instructions, &it->fn.binary_instructions_size))
            {
                return false;
            }

            break;
        default:
            fprintf(stderr, "ERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
            return false;
        }
    }

    return true;
}

static const char *gm_amd64_register_to_string(const gm_amd64_register_type_e register_type)
{
    static_assert(GM_AMD64_REGISTER_TYPE_GUARDIAN_ANGEL == 6, "gm_amd64_register_to_string(): AMD64 register type enum size changed");

    switch (register_type)
    {
    case GM_AMD64_REGISTER_TYPE_EAX:
        return "eax";
    case GM_AMD64_REGISTER_TYPE_EBX:
        return "ebx";
    case GM_AMD64_REGISTER_TYPE_ECX:
        return "ecx";
    case GM_AMD64_REGISTER_TYPE_EDX:
        return "edx";
    case GM_AMD64_REGISTER_TYPE_EDI:
        return "edi";
    case GM_AMD64_REGISTER_TYPE_ESI:
        return "esi";
    default:
        fprintf(stderr, "\tERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
        return "<unknown register>";
    }
}

static void gm_amd64_print_instructions(const gm_amd64_instruction_t *instructions)
{
    if (instructions == nullptr)
    {
        puts("\t<NO AMD64 INSTRUCTIONS>");
        return;
    }

    static_assert(GM_AMD64_INSTRUCTION_TYPE_GUARDIAN_ANGEL == 3, "gm_amd64_print_instructions(): AMD64 instruction type enum size changed");
    for (const gm_amd64_instruction_t *it = instructions; it != nullptr; it = it->next)
    {
        switch (it->type)
        {
        case GM_AMD64_INSTRUCTION_TYPE_MOV_REG_NUM:
            printf("\tmov %s, %" PRId32 "\n", gm_amd64_register_to_string(it->register_type), it->number);
            break;
        case GM_AMD64_INSTRUCTION_TYPE_RET:
            puts("\tret");
            break;
        case GM_AMD64_INSTRUCTION_TYPE_SYSCALL:
            puts("\tsyscall");
            break;
        default:
            fprintf(stderr, "\tERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
            return;
        }
    }
}

void gm_amd64_print_ast_tree(const gm_ast_node_t *ast_tree)
{
    if (ast_tree == nullptr)
    {
        puts("<NO AST>");
        return;
    }

    static_assert(GM_AST_NODE_TYPE_GUARDIAN_ANGEL == 1, "gm_amd64_print_ast_tree(): AST node type enum size changed");
    for (const gm_ast_node_t *it = ast_tree; it != nullptr; it = it->next)
    {
        switch (it->type)
        {
        case GM_AST_NODE_TYPE_FN:
            printf("%s:\n", it->fn.name.data);
            gm_amd64_print_instructions(it->fn.instructions);
            putchar('\n');
            break;
        default:
            fprintf(stderr, "ERROR: UNREACHABLE CODE: %s:%d\n", __FILE__, __LINE__);
            return;
        }
    }
}

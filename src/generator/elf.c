#include <gamma/generator/elf.h>
#include <stdio.h>
#include <string.h>

static uint16_t gm_u16_to_little_endian(const uint16_t value)
{
    union
    {
        uint16_t u16;
        uint8_t u8[2];
    } uni;

    uni.u8[0] = value & 0xFF;
    uni.u8[1] = (value >> 8) & 0xFF;

    return uni.u16;
}

static uint32_t gm_u32_to_little_endian(const uint32_t value)
{
    union
    {
        uint32_t u32;
        uint8_t u8[4];
    } uni;

    uni.u8[0] = value & 0xFF;
    uni.u8[1] = (value >> 8) & 0xFF;
    uni.u8[2] = (value >> 16) & 0xFF;
    uni.u8[3] = (value >> 24) & 0xFF;

    return uni.u32;
}

static uint64_t gm_u64_to_little_endian(const uint64_t value)
{
    union
    {
        uint64_t u64;
        uint8_t u8[8];
    } uni;

    uni.u8[0] = value & 0xFF;
    uni.u8[1] = (value >> 8) & 0xFF;
    uni.u8[2] = (value >> 16) & 0xFF;
    uni.u8[3] = (value >> 24) & 0xFF;
    uni.u8[4] = (value >> 32) & 0xFF;
    uni.u8[5] = (value >> 40) & 0xFF;
    uni.u8[6] = (value >> 48) & 0xFF;
    uni.u8[7] = (value >> 56) & 0xFF;

    return uni.u64;
}

static constexpr uint64_t gm_basic_virtual_address = 0x400000;

bool gm_amd64_generate_elf_header64(gm_arena_t *arena, const gm_object_file_type_e file_type, const gm_elf_header64_t **out)
{
    gm_elf_header64_t *header = gm_arena_alloc_single(arena, gm_elf_header64_t);
    if (header == nullptr)
    {
        fputs("ERROR: gm_amd64_generate_elf_header64(): allocation error\n", stderr);
        return false;
    }

    header->magic8[0] = 0x7F;
    header->magic8[1] = 'E';
    header->magic8[2] = 'L';
    header->magic8[3] = 'F';
    header->architecture = 2; // 64-bit
    header->endianness = 1; // Little endian
    header->version = 1; // Current version of ELF
    header->os_abi = 0x03; // Linux
    header->abi_version = 0; // Linux ignores it, glibc does weird things???

    switch (file_type)
    {
    case GM_OBJECT_FILE_TYPE_EXECUTABLE:
        header->type = gm_u16_to_little_endian(file_type);
        break;
    default:
        fputs("ERROR: gm_amd64_generate_elf_header64(): requested object file type is not yet supported", stderr);
        return false;
    }

    header->machine = gm_u16_to_little_endian(0x3E); // AMD64
    header->version_second = gm_u32_to_little_endian(1); // Current version of ELF
    header->entry = gm_u64_to_little_endian(gm_basic_virtual_address + sizeof(gm_elf_header64_t) + sizeof(gm_program_header64_t));
    header->program_header_offset = gm_u64_to_little_endian(0x40);
    header->section_header_offset = gm_u64_to_little_endian(0x00);
    header->flags = gm_u32_to_little_endian(0); // Don't know what it does XDDD
    header->elf_header_size = gm_u16_to_little_endian(sizeof(gm_elf_header64_t));
    header->program_header_entry_size = gm_u16_to_little_endian(sizeof(gm_program_header64_t));
    header->program_header_entry_count = gm_u16_to_little_endian(1);
    header->section_header_entry_size = gm_u16_to_little_endian(0x00);
    header->section_header_entry_count = gm_u16_to_little_endian(0x00);
    header->section_header_entry_index = gm_u16_to_little_endian(0x00);

    *out = header;
    return true;
}

static constexpr uint32_t gm_prot_read = 1 << 2;
// static constexpr uint32_t gm_prot_write = 1 << 1;
static constexpr uint32_t gm_prot_execute = 1 << 0;
static constexpr uint64_t page_alignment = 4096;

bool gm_amd64_generate_program_header64(gm_arena_t *arena, const uint64_t code_size, const gm_program_header64_t **out)
{
    gm_program_header64_t *header = gm_arena_alloc_single(arena, gm_program_header64_t);
    if (header == nullptr)
    {
        fputs("ERROR: gm_amd64_generate_program_header64(): allocation error\n", stderr);
        return false;
    }

    header->type = gm_u32_to_little_endian(0x01); // Loadable segment
    header->flags = gm_u32_to_little_endian(gm_prot_read | gm_prot_execute);
    header->offset = 0;
    header->virtual_address = gm_u64_to_little_endian(gm_basic_virtual_address);
    header->physical_address = gm_u64_to_little_endian(gm_basic_virtual_address);
    const uint64_t full_size = sizeof(gm_elf_header64_t) + sizeof(gm_program_header64_t) + code_size;
    header->file_size = full_size;
    header->memory_size = full_size;
    header->alignment = gm_u64_to_little_endian(page_alignment);

    *out = header;
    return true;
}

bool gm_amd64_generate_executable_image(gm_arena_t *arena, const gm_ast_node_t *ast_tree, const uint8_t **binary, size_t *binary_size)
{
    static_assert(GM_AST_NODE_TYPE_GUARDIAN_ANGEL == 1, "gm_amd64_generate_executable_image(): AST node type enum size changed");

    for (const gm_ast_node_t *it = ast_tree; it != nullptr; it = it->next)
    {
        if (it->type != GM_AST_NODE_TYPE_FN && strcmp(it->fn.name.data, "main") != 0)
        {
            continue;
        }

        const size_t full_size = sizeof(gm_elf_header64_t) + sizeof(gm_program_header64_t) + it->fn.binary_instructions_size;
        uint8_t *bytes = gm_arena_alloc(arena, full_size);
        if (bytes == nullptr)
        {
            fputs("ERROR: gm_amd64_generate_executable_image(): allocation error\n", stderr);
            return false;
        }

        const gm_elf_header64_t *elf;
        if (!gm_amd64_generate_elf_header64(arena, GM_OBJECT_FILE_TYPE_EXECUTABLE, &elf))
        {
            return false;
        }

        const gm_program_header64_t *prog;
        if (!gm_amd64_generate_program_header64(arena, it->fn.binary_instructions_size, &prog))
        {
            return false;
        }

        memcpy(bytes, elf, sizeof(gm_elf_header64_t));
        memcpy(bytes + sizeof(gm_elf_header64_t), prog, sizeof(gm_program_header64_t));
        memcpy(bytes + sizeof(gm_elf_header64_t) + sizeof(gm_program_header64_t), it->fn.binary_instructions, it->fn.binary_instructions_size);

        *binary = bytes;
        *binary_size = full_size;
        return true;
    }

    fputs("Linker Error: no \"main\" function provided\n", stderr);
    *binary = nullptr;
    *binary_size = 0;
    return false;
}

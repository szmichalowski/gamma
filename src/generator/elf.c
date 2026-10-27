#include <gamma/generator/elf.h>
#include <gamma/string/string.h>
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

static constexpr uint64_t gm_basic_virtual_address = 0;

bool gm_amd64_generate_elf_header64(gm_elf_header64_t *header, const gm_object_file_type_e file_type)
{
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
    case GM_OBJECT_FILE_TYPE_SHARED_OBJECT:
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
    header->section_header_entry_size = gm_u16_to_little_endian(sizeof(gm_section_header64_t));
    header->section_header_entry_count = gm_u16_to_little_endian(0x05);
    header->section_header_entry_index = gm_u16_to_little_endian(0x02);

    return true;
}

static constexpr uint32_t gm_prot_read = 1 << 2;
// static constexpr uint32_t gm_prot_write = 1 << 1;
static constexpr uint32_t gm_prot_execute = 1 << 0;
static constexpr uint64_t gm_page_alignment = 4096;

void gm_amd64_generate_program_header64(gm_program_header64_t *header, const uint64_t code_size)
{
    header->type = gm_u32_to_little_endian(0x01); // Loadable segment
    header->flags = gm_u32_to_little_endian(gm_prot_read | gm_prot_execute);
    header->offset = 0;
    header->virtual_address = gm_u64_to_little_endian(gm_basic_virtual_address);
    header->physical_address = gm_u64_to_little_endian(gm_basic_virtual_address);
    const uint64_t full_size = sizeof(gm_elf_header64_t) + sizeof(gm_program_header64_t) + code_size;
    header->file_size = full_size;
    header->memory_size = full_size;
    header->alignment = gm_u64_to_little_endian(gm_page_alignment);
}

void gm_amd64_generate_section_header64_text(gm_section_header64_t *header, const uint64_t code_size)
{
    header->name = gm_u32_to_little_endian(0x01);
    header->type = gm_u32_to_little_endian(0x01); // Program data
    header->flags = gm_u64_to_little_endian(0x02 + 0x04); // ALLOC + EXEC
    header->virtual_address = gm_u64_to_little_endian(sizeof(gm_elf_header64_t) + sizeof(gm_program_header64_t));
    header->offset = gm_u64_to_little_endian(sizeof(gm_elf_header64_t) + sizeof(gm_program_header64_t));
    header->size = code_size;
    header->alignment = gm_u64_to_little_endian(16);
}

void gm_amd64_generate_section_header64_sh_strings_table(gm_section_header64_t *header, const uint64_t code_size)
{
    header->name = gm_u32_to_little_endian(0x07);
    header->type = gm_u32_to_little_endian(0x03); // String table
    header->flags = gm_u64_to_little_endian(0x00);
    header->virtual_address = gm_u64_to_little_endian(0x00);
    header->offset = gm_u64_to_little_endian(sizeof(gm_elf_header64_t) + sizeof(gm_program_header64_t) + code_size);
    header->size = 33;
    header->alignment = gm_u64_to_little_endian(1);
}

static constexpr size_t gm_strings_sections_size = 40;
static constexpr uint8_t gm_strings_sections[gm_strings_sections_size] = "\0.text\0.shstrtab\0.symtab\0.strtab\0";

void gm_amd64_generate_section_header64_symbol_table(gm_section_header64_t *header, const gm_symbol_table64_entry_t *first_symtab_entry)
{
    header->name = gm_u32_to_little_endian(0x11);
    header->type = gm_u32_to_little_endian(0x02); // Symbol table
    header->flags = gm_u64_to_little_endian(0x00);
    header->virtual_address = gm_u64_to_little_endian(0x00);
    header->offset = gm_u64_to_little_endian((uint64_t)first_symtab_entry);
    header->size = 5 * sizeof(gm_symbol_table64_entry_t);
    header->link = gm_u32_to_little_endian(4);
    header->info = gm_u32_to_little_endian(1);
    header->alignment = gm_u64_to_little_endian(8);
    header->entry_size = sizeof(gm_symbol_table64_entry_t);
}

static constexpr size_t gm_preamble_size = 14;
static constexpr uint8_t gm_preamble[gm_preamble_size] = {
    0xE8, 0x09, 0x00, 0x00, 0x00, // call 0x400009 <main>
    0x89, 0xC7, // mov edi, eax
    0xB8, 0x3C, 0x00, 0x00, 0x00, // mov eax, 60
    0x0f, 0x05 // syscall
};
static constexpr size_t gm_preamble_call_main_offset = 1;
static constexpr size_t gm_preamble_after_call_size = 9;

void gm_amd64_generate_symbol_table64(gm_symbol_table64_entry_t *table, const gm_ast_node_t *ast_tree)
{
    static_assert(GM_AST_NODE_TYPE_GUARDIAN_ANGEL == 1, "gm_amd64_generate_executable_image(): AST node type enum size changed");

    // Zeroed out by memory arena
    // *table = (gm_symbol_table64_entry_t) { 0 };

    gm_symbol_table64_entry_t *preamble = table + 1;
    preamble->name = gm_u32_to_little_endian(1);
    preamble->info = 16 | 2; // GLOBAL | FUNC
    preamble->other = 0; // Default visibility
    preamble->section_header_index = 1;
    preamble->value = gm_u32_to_little_endian(sizeof(gm_elf_header64_t) + sizeof(gm_program_header64_t));
    preamble->size = gm_preamble_size;

    gm_symbol_table64_entry_t *table_entry = preamble + 1;
    // Right after _start
    uint32_t name = 8;
    uint64_t value = sizeof(gm_elf_header64_t) + sizeof(gm_program_header64_t) + gm_preamble_size;
    for (const gm_ast_node_t *it = ast_tree; it != nullptr; it = it->next)
    {
        if (it->type != GM_AST_NODE_TYPE_FN)
        {
            continue;
        }

        table_entry->name = name;
        table_entry->info = 16 | 2; // GLOBAL | FUNC
        table_entry->other = 0; // Default visibility
        table_entry->section_header_index = 1;
        table_entry->value = value;
        table_entry->size = it->fn.binary_instructions_size;

        ++table_entry;
        name += it->fn.name.size + 1;
        value += it->fn.binary_instructions_size;
    }
}

void gm_amd64_generate_section_header64_strings_table(gm_section_header64_t *header, const uint64_t code_size, const uint64_t str_size)
{
    header->name = gm_u32_to_little_endian(0x19);
    header->type = gm_u32_to_little_endian(0x03); // String table
    header->flags = gm_u64_to_little_endian(0x00);
    header->virtual_address = gm_u64_to_little_endian(0x00);
    header->offset = gm_u64_to_little_endian(sizeof(gm_elf_header64_t) + sizeof(gm_program_header64_t) + code_size + gm_strings_sections_size);
    header->size = str_size;
    header->link = gm_u32_to_little_endian(0);
    header->info = gm_u32_to_little_endian(0);
    header->alignment = gm_u64_to_little_endian(1);
    header->entry_size = 0;
}

bool gm_amd64_generate_executable_image(gm_arena_t *arena, const gm_ast_node_t *ast_tree, const uint8_t **binary, size_t *binary_size)
{
    static_assert(GM_AST_NODE_TYPE_GUARDIAN_ANGEL == 1, "gm_amd64_generate_executable_image(): AST node type enum size changed");

    size_t num_funcs = 0;
    gm_string_t strings_symtab;
    gm_string_init(arena, &strings_symtab);
    gm_string_append_char(arena, &strings_symtab, 0);
    gm_string_append_strlit(arena, &strings_symtab, "_start");
    gm_string_append_char(arena, &strings_symtab, 0);
    for (const gm_ast_node_t *it = ast_tree; it != nullptr; it = it->next)
    {
        if (it->type != GM_AST_NODE_TYPE_FN)
        {
            continue;
        }

        ++num_funcs;
        gm_string_append_string(arena, &strings_symtab, it->fn.name);
        gm_string_append_char(arena, &strings_symtab, 0);
    }
    const size_t strings_symtab_size = (strings_symtab.size + 7) & ~7;

    size_t alloc_size = sizeof(gm_elf_header64_t) + sizeof(gm_program_header64_t) + gm_preamble_size;
    uint8_t *bytes = gm_arena_alloc(arena, alloc_size);
    memcpy(bytes + sizeof(gm_elf_header64_t) + sizeof(gm_program_header64_t), gm_preamble, gm_preamble_size);

    bool main_encountered = false;
    uint8_t *preamble_call_main;
    uint32_t main_offset_le;
    for (const gm_ast_node_t *it = ast_tree; it != nullptr; it = it->next)
    {
        if (it->type != GM_AST_NODE_TYPE_FN)
        {
            continue;
        }

        const size_t new_size = alloc_size + it->fn.binary_instructions_size;
        bytes = gm_arena_realloc(arena, bytes, alloc_size, new_size);

        if (strcmp(it->fn.name.data, "main") == 0)
        {
            preamble_call_main = bytes + sizeof(gm_elf_header64_t) + sizeof(gm_program_header64_t) + gm_preamble_call_main_offset;
            const uint32_t main_offset = alloc_size - sizeof(gm_elf_header64_t) - sizeof(gm_program_header64_t) - gm_preamble_size + gm_preamble_after_call_size;
            main_offset_le = gm_u32_to_little_endian(main_offset);
            main_encountered = true;
        }

        memcpy(bytes + alloc_size, it->fn.binary_instructions, it->fn.binary_instructions_size);
        alloc_size = new_size;
    }

    if (!main_encountered)
    {
        fputs("Linker Error: no \"main\" function provided\n", stderr);
        *binary = nullptr;
        *binary_size = 0;
        return false;
    }

    gm_elf_header64_t *elf = (gm_elf_header64_t *)bytes;
    if (!gm_amd64_generate_elf_header64(elf, GM_OBJECT_FILE_TYPE_SHARED_OBJECT))
    {
        return false;
    }

    gm_program_header64_t *prog = (gm_program_header64_t *)(bytes + sizeof(gm_elf_header64_t));
    gm_amd64_generate_program_header64(prog, alloc_size - sizeof(gm_elf_header64_t) - sizeof(gm_program_header64_t));
    preamble_call_main[0] = main_offset_le & 0xFF;
    preamble_call_main[1] = (main_offset_le >> 8) & 0xFF;
    preamble_call_main[2] = (main_offset_le >> 16) & 0xFF;
    preamble_call_main[3] = (main_offset_le >> 24) & 0xFF;

    const size_t new_size = alloc_size + gm_strings_sections_size + strings_symtab_size + (num_funcs + 2) * sizeof(gm_symbol_table64_entry_t) + 5 * sizeof(gm_section_header64_t);
    bytes = gm_arena_realloc(arena, bytes, alloc_size, new_size);

    uint8_t *strings = bytes + alloc_size;
    memcpy(strings, gm_strings_sections, gm_strings_sections_size);

    uint8_t *strtab = strings + gm_strings_sections_size;
    memcpy(strtab, strings_symtab.data, strings_symtab_size);

    gm_symbol_table64_entry_t *first_symtab_entry = (gm_symbol_table64_entry_t *)(strtab + strings_symtab_size);
    gm_amd64_generate_symbol_table64(first_symtab_entry, ast_tree);

    const uint64_t code_size = alloc_size - sizeof(gm_elf_header64_t) - sizeof(gm_program_header64_t);

    gm_section_header64_t *text_header = (gm_section_header64_t *)(first_symtab_entry + num_funcs + 2) + 1;
    gm_amd64_generate_section_header64_text(text_header, code_size);

    gm_section_header64_t *strings_header = text_header + 1;
    gm_amd64_generate_section_header64_sh_strings_table(strings_header, code_size);

    gm_section_header64_t *symtab_header = strings_header + 1;
    gm_amd64_generate_section_header64_symbol_table(symtab_header, (gm_symbol_table64_entry_t *)((uint8_t *)first_symtab_entry - bytes));

    gm_section_header64_t *strings_table = symtab_header + 1;
    gm_amd64_generate_section_header64_strings_table(strings_table, code_size, strings_symtab.size);

    elf->section_header_offset = (uint64_t)alloc_size + (uint64_t)gm_strings_sections_size + strings_symtab_size + (num_funcs + 2) * 24;
    alloc_size = new_size;

    *binary = bytes;
    *binary_size = alloc_size;
    return true;
}

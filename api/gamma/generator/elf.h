#ifndef GM_API_GENERATOR_ELF_H
#define GM_API_GENERATOR_ELF_H

#include <gamma/arena/arena.h>
#include <gamma/parser/parser.h>
#include <stdint.h>

typedef struct [[gnu::packed]] gm_elf_header64
{
    union
    {
        uint32_t magic32;
        uint8_t magic8[4];
    };
    uint8_t architecture;
    uint8_t endianness;
    uint8_t version;
    uint8_t os_abi;
    uint8_t abi_version;
    uint8_t padding[7];
    uint16_t type;
    uint16_t machine;
    uint32_t version_second;
    uint64_t entry;
    uint64_t program_header_offset;
    uint64_t section_header_offset;
    uint32_t flags;
    uint16_t elf_header_size;
    uint16_t program_header_entry_size;
    uint16_t program_header_entry_count;
    uint16_t section_header_entry_size;
    uint16_t section_header_entry_count;
    uint16_t section_header_entry_index;
} gm_elf_header64_t;
static_assert(sizeof(gm_elf_header64_t) == 64, "64-bit ELF header must be 64 (0x40) bytes long");

typedef enum gm_object_file_type : uint16_t
{
    GM_OBJECT_FILE_TYPE_UNKNOWN = 0x00,
    GM_OBJECT_FILE_TYPE_RELOCATABLE = 0x01,
    GM_OBJECT_FILE_TYPE_EXECUTABLE = 0x02,
    GM_OBJECT_FILE_TYPE_SHARED_OBJECT = 0x03,
    GM_OBJECT_FILE_TYPE_CORE = 0x04,

    GM_OBJECT_FILE_TYPE_OS_LOW = 0xFE00,
    GM_OBJECT_FILE_TYPE_OS_HIGH = 0xFEFF,
    GM_OBJECT_FILE_TYPE_PROCESSOR_LOW = 0xFF00,
    GM_OBJECT_FILE_TYPE_PROCESSOR_HIGH = 0xFFFF
} gm_object_file_type_e;

typedef struct [[gnu::packed]] gm_program_header64
{
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t virtual_address;
    uint64_t physical_address;
    uint64_t file_size;
    uint64_t memory_size;
    uint64_t alignment;
} gm_program_header64_t;
static_assert(sizeof(gm_program_header64_t) == 56, "64-bit ELF program header must be 56 (0x38) bytes long");

typedef struct [[gnu::packed]] gm_section_header64
{
    uint32_t name;
    uint32_t type;
    uint64_t flags;
    uint64_t virtual_address;
    uint64_t offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t alignment;
    uint64_t entry_size;
} gm_section_header64_t;
static_assert(sizeof(gm_section_header64_t) == 64, "64-bit ELF section header must be 64 (0x40) bytes long");

typedef struct [[gnu::packed]] gm_symbol_table64_entry
{
    uint32_t name;
    uint8_t info;
    uint8_t other;
    uint16_t section_header_index;
    uint64_t value;
    uint64_t size;
} gm_symbol_table64_entry_t;
static_assert(sizeof(gm_symbol_table64_entry_t) == 24, "64-bit ELF symbol table entry must be 24 (0x18) bytes long");

bool gm_amd64_generate_elf_header64(gm_elf_header64_t *header, const gm_object_file_type_e file_type);
void gm_amd64_generate_program_header64(gm_program_header64_t *header, uint64_t code_size);

void gm_amd64_generate_section_header64_text(gm_section_header64_t *header, uint64_t code_size);
void gm_amd64_generate_section_header64_sh_strings_table(gm_section_header64_t *header, uint64_t code_size);
void gm_amd64_generate_section_header64_symbol_table(gm_section_header64_t *header, const gm_symbol_table64_entry_t *first_symtab_entry);
void gm_amd64_generate_section_header64_strings_table(gm_section_header64_t *header, uint64_t code_size, uint64_t str_size);

void gm_amd64_generate_symbol_table64(gm_symbol_table64_entry_t *table, const gm_ast_node_t *ast_tree);

bool gm_amd64_generate_executable_image(gm_arena_t *arena, const gm_ast_node_t *ast_tree, const uint8_t **binary, size_t *binary_size);

#endif // GM_API_GENERATOR_ELF_H

#include <gamma/generator/elf.h>
#include <stdio.h>

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

bool gm_amd64_generate_elf64_header(gm_arena_t *arena, const gm_object_file_type_e file_type, const gm_elf64_header_t **out)
{
    gm_elf64_header_t *header = gm_arena_alloc_single(arena, gm_elf64_header_t);
    if (header == nullptr)
    {
        fputs("ERROR: gm_amd64_generate_elf64_header(): allocation error\n", stderr);
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
        fputs("ERROR: gm_amd64_generate_elf64_header(): requested object file type is not yet supported", stderr);
        return false;
    }

    header->machine = gm_u16_to_little_endian(0x3E); // AMD64
    header->version_second = gm_u32_to_little_endian(1); // Current version of ELF
    header->entry = gm_u64_to_little_endian(0x400078);
    header->program_header_offset = gm_u64_to_little_endian(0x40);
    header->section_header_offset = gm_u64_to_little_endian(0x00);
    header->flags = gm_u32_to_little_endian(0); // Don't know what it does XDDD
    header->elf_header_size = gm_u16_to_little_endian(sizeof *header);
    // TODO: Make it understandable
    header->program_header_entry_size = gm_u16_to_little_endian(0x38);
    header->program_header_entry_count = gm_u16_to_little_endian(0x01);
    header->section_header_entry_size = gm_u16_to_little_endian(0x00);
    header->section_header_entry_count = gm_u16_to_little_endian(0x00);
    header->section_header_entry_index = gm_u16_to_little_endian(0x00);

    *out = header;
    return true;
}

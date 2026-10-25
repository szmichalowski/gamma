#ifndef GM_API_ARENA_ARENA_H
#define GM_API_ARENA_ARENA_H

#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>

#define GM_KB(x) ((size_t)(x) << 10)
#define GM_MB(x) ((size_t)(x) << 20)
#define GM_GB(x) ((size_t)(x) << 30)

typedef struct gm_arena
{
    uint8_t *base;
    size_t reserved;
    size_t commited;
    size_t offset;
    jmp_buf *longjmp_buffer;
    int longjmp_ret;
} gm_arena_t;

bool gm_arena_init(gm_arena_t *arena, size_t reserve_size);
void gm_arena_destroy(gm_arena_t *arena);

void gm_arena_set_callback(gm_arena_t *arena, jmp_buf *longjmp_buffer, int longjmp_ret);

void *gm_arena_alloc_aligned(gm_arena_t *arena, size_t size, size_t align);
void *gm_arena_realloc_aligned(gm_arena_t *arena, void *old_ptr, size_t old_size, size_t new_size, size_t align);

void *gm_arena_alloc_aligned_ext(gm_arena_t *arena, size_t size, size_t align, const char *source_file, size_t source_line);
void *gm_arena_realloc_aligned_ext(gm_arena_t *arena, void *old_ptr, size_t old_size, size_t new_size, size_t align, const char *source_file, size_t source_line);

void gm_arena_reset(gm_arena_t *arena);
void gm_arena_hard_reset(gm_arena_t *arena);

typedef struct gm_arena_checkpoint
{
    gm_arena_t *arena;
    size_t offset;
} gm_arena_checkpoint_t;

gm_arena_checkpoint_t gm_arena_checkpoint_begin(gm_arena_t *arena);
bool gm_arena_checkpoint_end(gm_arena_checkpoint_t *checkpoint);
bool gm_arena_checkpoint_hard_end(gm_arena_checkpoint_t *checkpoint);

#define gm_arena_alloc_single(arena, T) (T *)gm_arena_alloc_aligned_ext((arena), sizeof(T), alignof(T), __FILE__, __LINE__)
#define gm_arena_alloc_array(arena, T, count) (T *)gm_arena_alloc_aligned_ext((arena), sizeof(T) * (count), alignof(T), __FILE__, __LINE__)
#define gm_arena_alloc(arena, size) gm_arena_alloc_aligned_ext((arena), (size), alignof(max_align_t), __FILE__, __LINE__)

#define gm_arena_realloc_array(arena, T, old_ptr, old_count, new_count) (T *)gm_arena_realloc_aligned_ext((arena), (old_ptr), sizeof(T) * (old_count), sizeof(T) * (new_count), alignof(T), __FILE__, __LINE__)
#define gm_arena_realloc(arena, old_ptr, old_size, new_size) gm_arena_realloc_aligned_ext((arena), (old_ptr), (old_size), (new_size), alignof(max_align_t), __FILE__, __LINE__)

#endif // GM_API_ARENA_ARENA_H

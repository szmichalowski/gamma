#include <gamma/arena/arena.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32

#error "Windows is currently not supported"

#else // _WIN32

#include <sys/mman.h>
#include <unistd.h>

static size_t gm_os_page_size()
{
    return (size_t)sysconf(_SC_PAGESIZE);
}

static void *gm_os_mem_reserve(size_t size)
{
    void *ptr = mmap(nullptr, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return ptr == MAP_FAILED ? nullptr : ptr;
}

static bool gm_os_mem_commit(void *ptr, size_t size)
{
    if (mprotect(ptr, size, PROT_READ | PROT_WRITE) != 0)
    {
        return false;
    }

#ifdef MADV_HUGEPAGE
    madvise(ptr, size, MADV_HUGEPAGE);
#endif // MADV_HUGEPAGE

    return true;
}

static void gm_os_mem_decommit(void *ptr, size_t size)
{
#if defined(MADV_FREE) && defined(__APPLE__)
    madvise(ptr, size, MADV_FREE);
#else // defined(MADV_FREE) && defined(__APPLE__)
    madvise(ptr, size, MADV_DONTNEED);
#endif // defined(MADV_FREE) && defined(__APPLE__)

    mprotect(ptr, size, PROT_NONE);
}

static void gm_os_mem_release(void *ptr, size_t size)
{
    munmap(ptr, size);
}

#endif // _WIN32

#define GM_ARENA_DEFAULT_RESERVE_SIZE GM_GB(2)
#define GM_ARENA_COMMIT_GRANULARITY GM_MB(1)

static uintptr_t gm_align_forward_uintptr(const uintptr_t ptr, const size_t align)
{
    if ((align & (align - 1)) != 0)
    {
        fputs("ERROR: gm_align_forward_uintptr(): alignment must be a power of 2\n", stderr);
        return 0;
    }

    return (ptr + (align - 1)) & ~(uintptr_t)(align - 1);
}

static size_t gm_align_forward_size(const size_t size, const size_t align)
{
    if ((align & (align - 1)) != 0)
    {
        fputs("ERROR: gm_align_forward_size(): alignment must be a power of 2\n", stderr);
        return 0;
    }

    return (size + (align - 1)) & ~(align - 1);
}

bool gm_arena_init(gm_arena_t *arena, size_t reserve_size)
{
    reserve_size = reserve_size > 0 ? reserve_size : GM_ARENA_DEFAULT_RESERVE_SIZE;
    reserve_size = gm_align_forward_size(reserve_size, gm_os_page_size());

    void *base = gm_os_mem_reserve(reserve_size);
    if (base == nullptr)
    {
        return false;
    }

    arena->base = (uint8_t *)base;
    arena->reserved = reserve_size;
    arena->commited = 0;
    arena->offset = 0;

    return true;
}

void gm_arena_destroy(gm_arena_t *arena)
{
    if (arena->base != nullptr)
    {
        gm_os_mem_release(arena->base, arena->reserved);
    }

    *arena = (gm_arena_t) { 0 };
}

static bool gm_arena_ensure_commited(gm_arena_t *arena, const size_t required_offset)
{
    if (required_offset > arena->reserved)
    {
        return false;
    }

    if (required_offset > arena->commited)
    {
        const size_t granularity = gm_align_forward_size(GM_ARENA_COMMIT_GRANULARITY, gm_os_page_size());
        size_t new_commited = gm_align_forward_size(required_offset, granularity);
        new_commited = new_commited <= arena->reserved ? new_commited : arena->reserved;
        const size_t commit_delta = new_commited - arena->commited;
        if (!gm_os_mem_commit(arena->base + arena->commited, commit_delta))
        {
            return false;
        }
        arena->commited = new_commited;
    }

    return true;
}

void *gm_arena_alloc_aligned(gm_arena_t *arena, const size_t size, const size_t align)
{
    if (arena->base == nullptr)
    {
        if (!gm_arena_init(arena, GM_ARENA_DEFAULT_RESERVE_SIZE))
        {
            return nullptr;
        }
    }

    const uintptr_t curr_ptr = (uintptr_t)arena->base + arena->offset;
    const uintptr_t aligned_ptr = gm_align_forward_uintptr(curr_ptr, align);
    const size_t padding = aligned_ptr - curr_ptr;
    const size_t new_offset = arena->offset + padding + size;

    if (!gm_arena_ensure_commited(arena, new_offset))
    {
        return nullptr;
    }

    arena->offset = new_offset;
    void *result = (void *)aligned_ptr;

    memset(result, 0, size);
    return result;
}

void *gm_arena_realloc_aligned(gm_arena_t *arena, void *old_ptr, const size_t old_size, const size_t new_size, const size_t align)
{
    if (old_ptr == nullptr || old_size == 0)
    {
        return gm_arena_alloc_aligned(arena, new_size, align);
    }

    uint8_t *old_bytes = old_ptr;
    if (old_bytes < arena->base || old_bytes > arena->base + arena->offset)
    {
        fputs("ERROR: gm_align_forward_size(): input pointer outside of the arena\n", stderr);
        return nullptr;
    }

    if (old_bytes + old_size == arena->base + arena->offset)
    {
        const size_t base_offset = (size_t)(old_bytes - arena->base);
        const size_t new_offset = base_offset + new_size;

        if (!gm_arena_ensure_commited(arena, new_offset))
        {
            return nullptr;
        }

        if (new_size > old_size)
        {
            memset(old_bytes + old_size, 0, new_size - old_size);
        }

        arena->offset = new_offset;
        return old_ptr;
    }

    if (new_size <= old_size)
    {
        return old_ptr;
    }

    void *new_ptr = gm_arena_alloc_aligned(arena, new_size, align);
    if (new_ptr != nullptr)
    {
        memcpy(new_ptr, old_ptr, old_size);
    }
    return new_ptr;
}

void gm_arena_reset(gm_arena_t *arena)
{
    arena->offset = 0;
}

void gm_arena_hard_reset(gm_arena_t *arena)
{
    arena->offset = 0;

    if (arena->commited > 0)
    {
        gm_os_mem_decommit(arena->base, arena->commited);
        arena->commited = 0;
    }
}

gm_arena_checkpoint_t gm_arena_checkpoint_begin(gm_arena_t *arena)
{
    return (gm_arena_checkpoint_t) {
        .arena = arena,
        .offset = arena->offset
    };
}

bool gm_arena_checkpoint_end(gm_arena_checkpoint_t *checkpoint)
{
    if (checkpoint->offset > checkpoint->arena->offset)
    {
        fputs("ERROR: gm_arena_checkpoint_end(): input pointer outside of the arena\n", stderr);
        return false;
    }
    checkpoint->arena->offset = checkpoint->offset;

    return true;
}

bool gm_arena_checkpoint_hard_end(gm_arena_checkpoint_t *checkpoint)
{
    if (checkpoint->offset > checkpoint->arena->offset)
    {
        fputs("ERROR: gm_arena_checkpoint_end(): input pointer outside of the arena\n", stderr);
        return false;
    }
    checkpoint->arena->offset = checkpoint->offset;

    const size_t granularity = gm_align_forward_size(GM_ARENA_COMMIT_GRANULARITY, gm_os_page_size());
    const size_t target_commited = gm_align_forward_size(checkpoint->offset, granularity);

    if (target_commited < checkpoint->arena->commited)
    {
        const size_t decommit_size = checkpoint->arena->commited - target_commited;
        gm_os_mem_decommit(checkpoint->arena->base + target_commited, decommit_size);
        checkpoint->arena->commited = target_commited;
    }

    return true;
}

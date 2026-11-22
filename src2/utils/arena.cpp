#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <gamma/utils/arena.hpp>
#include <new>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <malloc.h>
#include <windows.h>
#else
#include <sched.h>
#include <sys/mman.h>
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif

namespace
{

static_assert(sizeof(void *) == 8, "Multi-gigbyte arena requires 64-bit architecture");

static_assert((gm::utils::ARENA_COMMIT_CHUNK & (gm::utils::ARENA_COMMIT_CHUNK - 1)) == 0, "ARENA_COMMIT_CHUNK must be a power of 2");
static_assert(gm::utils::ARENA_CAPACITY % gm::utils::ARENA_COMMIT_CHUNK == 0, "ARENA_CAPACITY must be divisible by ARENA_COMMIT_CHUNK");
static_assert(std::atomic<std::uintptr_t>::is_always_lock_free);
static_assert(std::atomic<std::size_t>::is_always_lock_free);

inline void *reserve_memory(std::size_t size) noexcept
{
#ifdef _WIN32
    return ::VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_NOACCESS);
#else
    int flags { MAP_PRIVATE | MAP_ANONYMOUS };
#ifdef MAP_NORESERVE
    flags |= MAP_NORESERVE;
#endif
    void *p { ::mmap(nullptr, size, PROT_NONE, flags, -1, 0) };
    return p == MAP_FAILED ? nullptr : p;
#endif
}

inline bool commit_memory(void *addr, std::size_t size) noexcept
{
#ifdef _WIN32
    return ::VirtualAlloc(addr, size, MEM_COMMIT, PAGE_READWRITE) != nullptr;
#else
    return ::mprotect(addr, size, PROT_READ | PROT_WRITE) == 0;
#endif
}

inline void thread_yield() noexcept
{
#ifdef _WIN32
    ::SwitchToThread();
#else
    ::sched_yield();
#endif
}

inline constexpr std::uintptr_t UNINIT { 0 };
inline constexpr std::uintptr_t INIT_IN_PROGRESS { 1 };
inline constexpr std::uintptr_t INIT_FAILED { 2 };

constinit std::atomic<std::uintptr_t> base_addr { UNINIT };
constinit std::atomic<std::size_t> offset { 0 };
constinit std::atomic<std::size_t> committed { 0 };
constinit std::atomic_flag commit_lock {};

inline std::uintptr_t init_arena() noexcept
{
    std::uintptr_t expected { UNINIT };
    if (base_addr.compare_exchange_strong(expected, INIT_IN_PROGRESS, std::memory_order_acq_rel, std::memory_order_acquire))
    {
        void *mem { ::reserve_memory(gm::utils::ARENA_CAPACITY) };
        const std::uintptr_t addr { mem ? reinterpret_cast<std::uintptr_t>(mem) : INIT_FAILED };
        base_addr.store(addr, std::memory_order_release);
        return addr;
    }

    while (expected == INIT_IN_PROGRESS)
    {
        ::thread_yield();
        expected = base_addr.load(std::memory_order_acquire);
    }
    return expected;
}

inline bool ensure_committed(std::uintptr_t base, std::size_t needed) noexcept
{
    if (needed <= committed.load(std::memory_order_acquire)) [[likely]]
    {
        return true;
    }

    while (commit_lock.test_and_set(std::memory_order_acquire))
    {
        ::thread_yield();
    }

    bool ok { true };
    const std::size_t cur_comm { committed.load(std::memory_order_relaxed) };
    if (needed > cur_comm)
    {
        const std::size_t target { (needed + gm::utils::ARENA_COMMIT_CHUNK - 1) & ~(gm::utils::ARENA_COMMIT_CHUNK - 1) };
        const std::size_t delta { target - cur_comm };
        ok = ::commit_memory(reinterpret_cast<void *>(base + cur_comm), delta);
        if (ok)
        {
            committed.store(target, std::memory_order_release);
        }
    }

    commit_lock.clear(std::memory_order_release);
    return ok;
}

inline void *allocate(std::size_t size, std::size_t align) noexcept
{
    if (size == 0)
    {
        size = 1;
    }
    if (align == 0 || (align & (align - 1)) != 0)
    {
        return nullptr;
    }

    std::uintptr_t base { base_addr.load(std::memory_order_acquire) };
    if (base <= INIT_FAILED) [[unlikely]]
    {
        base = ::init_arena();
        if (base <= INIT_FAILED)
        {
            return nullptr;
        }
    }

    std::size_t cur { offset.load(std::memory_order_relaxed) };
    while (true)
    {
        const std::uintptr_t cur_addr { base + cur };
        const std::uintptr_t aligned_addr { (cur_addr + (align - 1)) & ~(align - 1) };
        const std::size_t padding { aligned_addr - cur_addr };

        if (aligned_addr < cur_addr || size > gm::utils::ARENA_CAPACITY || cur > gm::utils::ARENA_CAPACITY - size || padding > (gm::utils::ARENA_CAPACITY - size - cur))
        {
            return nullptr;
        }

        const std::size_t next { cur + padding + size };
        if (offset.compare_exchange_weak(cur, next, std::memory_order_relaxed, std::memory_order_relaxed))
        {
            if (!::ensure_committed(base, next)) [[unlikely]]
            {
                return nullptr;
            }
            return reinterpret_cast<void *>(aligned_addr);
        }
    }
}

inline bool owns(const void *ptr) noexcept
{
    const std::uintptr_t base { base_addr.load(std::memory_order_acquire) };
    if (base <= INIT_FAILED)
    {
        return false;
    }
    const auto addr { reinterpret_cast<std::uintptr_t>(ptr) };
    return addr >= base && addr < base + gm::utils::ARENA_CAPACITY;
}

void *alloc_impl(std::size_t size, std::size_t align, bool aligned_overload)
{
    if (void *p { ::allocate(size, align) })
    {
        return p;
    }

    if constexpr (!gm::utils::ARENA_FALLBACK_TO_HEAP)
    {
        throw std::bad_alloc {};
    }
    else
    {
        if (size == 0)
        {
            size = 1;
        }
        void *p { nullptr };
        if (!aligned_overload)
        {
            p = std::malloc(size);
        }
        else
        {
#ifdef _MSC_VER
            p = _aligned_malloc(size, align);
#else
            if (align < sizeof(void *))
            {
                align = sizeof(void *);
            }
            std::size_t padded { (size + align - 1) & ~(align - 1) };
            p = std::aligned_alloc(align, padded);
#endif
        }
        if (!p)
        {
            throw std::bad_alloc {};
        }
        return p;
    }
}

void free_impl(void *ptr, bool aligned_overload) noexcept
{
    if constexpr (!gm::utils::ARENA_FALLBACK_TO_HEAP)
    {
        return;
    }
    else
    {
        if (!ptr || ::owns(ptr))
        {
            return;
        }
        if (!aligned_overload)
        {
            std::free(ptr);
        }
        else
        {
#ifdef _MSC_VER
            _aligned_free(ptr);
#else
            std::free(ptr);
#endif
        }
    }
}

} // anonymous namespace

void *operator new(std::size_t size)
{
    return ::alloc_impl(size, __STDCPP_DEFAULT_NEW_ALIGNMENT__, false);
}

void *operator new(std::size_t size, std::align_val_t align)
{
    return ::alloc_impl(size, static_cast<std::size_t>(align), true);
}

void operator delete(void *ptr) noexcept
{
    ::free_impl(ptr, false);
}

void operator delete(void *ptr, std::size_t) noexcept
{
    ::free_impl(ptr, false);
}

void operator delete(void *ptr, std::align_val_t) noexcept
{
    ::free_impl(ptr, true);
}

void operator delete(void *ptr, std::size_t, std::align_val_t) noexcept
{
    ::free_impl(ptr, true);
}

inline std::size_t used() noexcept { return offset.load(std::memory_order_relaxed); }
inline std::size_t committed_bytes() noexcept { return committed.load(std::memory_order_acquire); }

namespace gm::utils
{

std::size_t arena_used() noexcept
{
    return offset.load(std::memory_order_relaxed);
}

std::size_t arena_committed_bytes() noexcept
{
    return committed.load(std::memory_order_acquire);
}

}

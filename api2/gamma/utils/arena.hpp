#pragma once

#include <cstddef>

namespace gamma::utils
{

inline constexpr std::size_t ARENA_CAPACITY { 4z * 1024z * 1024z * 1024z }; // 4GB
inline constexpr std::size_t ARENA_COMMIT_CHUNK { 16z * 1024z * 1024z }; // 16MB
inline constexpr bool ARENA_FALLBACK_TO_HEAP { false };

std::size_t arena_used() noexcept;
std::size_t arena_committed_bytes() noexcept;

}

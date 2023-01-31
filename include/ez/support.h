#ifndef EZ_SUPPORT_H
#define EZ_SUPPORT_H

#include <cstddef>
#include <cstdint>

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#ifndef __has_feature
#define __has_feature(x) 0
#endif

#if __has_builtin(__builtin_unreachable)
# define EZ_BUILTIN_UNREACHABLE __builtin_unreachable()
#elif (__GNUC__ >= 4 && __GNUC_MINOR__ >= 5)
# define EZ_BUILTIN_UNREACHABLE __builtin_unreachable()
#else
# define EZ_BUILTIN_UNREACHABLE
#endif

#if __has_feature(cxx_attributes)
# define EZ_NORETURN [[noreturn]]
#elif defined(__GNUC__)
# define EZ_NORETURN __attribute__((noreturn))
#else
# define EZ_NORETURN
#endif

inline char *addr2ptr(uint32_t Addr) {
  return reinterpret_cast<char *>(static_cast<uintptr_t>(Addr));
}

template <typename T>
uint32_t ptr2addr(T *Ptr) {
  return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(Ptr));
}

// Return true if the argument is a power of two > 0
constexpr inline bool is_power_of_2(uint32_t Value) {
  return Value && !(Value & (Value - 1));
}

template <uint32_t Align, typename T>
static T *align_ptr(T *Ptr) {
  constexpr uint32_t Mask = Align - 1;
  static_assert(is_power_of_2(Align), "Alignment must be a power of 2");
  return addr2ptr(ptr2addr(Ptr + Mask) & ~Mask);
}

// Determine the size of a C array at compile-time
template <typename T, size_t sizeOfArray>
constexpr unsigned c_array_size(T (&)[sizeOfArray]) {
  return sizeOfArray;
}

#endif // EZ_SUPPORT_H

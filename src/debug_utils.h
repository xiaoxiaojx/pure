#ifndef SRC_DEBUG_UTILS_H_
#define SRC_DEBUG_UTILS_H_
#include <string>

// Use FORCE_INLINE on functions that have a debug-category-enabled check first
// and then ideally only a single function call following it, to maintain
// performance for the common case (no debugging used).
#ifdef __GNUC__
#define FORCE_INLINE __attribute__((always_inline))
#define COLD_NOINLINE __attribute__((cold, noinline))
#else
#define FORCE_INLINE
#define COLD_NOINLINE
#endif

namespace pure {
template <typename... Args>
inline std::string SPrintF(const char* format, Args&&... args);
}

#endif  // SRC_DEBUG_UTILS_H_

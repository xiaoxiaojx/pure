#ifndef SRC_DEBUG_UTILS_INL_H_
#define SRC_DEBUG_UTILS_INL_H_

#include "debug_utils.h"
#include "util.h"

namespace pure {

inline std::string SPrintFImpl(const char* format) {
  const char* p = strchr(format, '%');
  if (LIKELY(p == nullptr)) return format;
  CHECK_EQ(p[1], '%');  // Only '%%' allowed when there are no arguments.

  return std::string(format, p + 1) + SPrintFImpl(p + 2);
}

template <typename... Args>
std::string COLD_NOINLINE SPrintF(  // NOLINT(runtime/string)
    const char* format,
    Args&&... args) {
  return SPrintFImpl(format, std::forward<Args>(args)...);
}

}  // namespace pure

#endif  // SRC_DEBUG_UTILS_INL_H_

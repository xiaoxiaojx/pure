#ifndef SRC_BASE64_H_
#define SRC_BASE64_H_

#include "util.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace pure {

// Doesn't check for padding at the end.  Can be 1-2 bytes over.
static inline constexpr size_t base64_decoded_size_fast(size_t size) {
  // 1-byte input cannot be decoded
  return size > 1 ? (size / 4) * 3 + (size % 4 + 1) / 2 : 0;
}
}  // namespace pure

#endif  // SRC_BASE64_H_

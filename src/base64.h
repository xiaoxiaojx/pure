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

template <typename TypeName>
size_t base64_decoded_size(const TypeName* src, size_t size) {
  // 1-byte input cannot be decoded
  if (size < 2) return 0;

  if (src[size - 1] == '=') {
    size--;
    if (src[size - 1] == '=') size--;
  }
  return base64_decoded_size_fast(size);
}
}  // namespace pure

#endif  // SRC_BASE64_H_

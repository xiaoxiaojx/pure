#ifndef SRC_STRING_BYTES_H_
#define SRC_STRING_BYTES_H_

#include "env-inl.h"
#include "v8.h"

#include <string>

namespace pure {

class StringBytes {
 public:
  // Fast, but can be 2 bytes oversized for Base64, and
  // as much as triple UTF-8 strings <= 65536 chars in length
  static v8::Maybe<size_t> StorageSize(v8::Isolate* isolate,
                                       v8::Local<v8::Value> val,
                                       enum encoding enc);
};

}  // namespace pure

#endif  // SRC_STRING_BYTES_H_

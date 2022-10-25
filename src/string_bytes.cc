#include "string_bytes.h"

#include <climits>
#include "base64.h"
#include "env-inl.h"
#include "pure_errors.h"
#include "util.h"

#include <algorithm>

namespace pure {

using v8::HandleScope;
using v8::Isolate;
using v8::Just;
using v8::Local;
using v8::Maybe;
using v8::MaybeLocal;
using v8::Nothing;
using v8::String;
using v8::Value;

// StorageSize 仅估算出最大可能的大小, 比如 UTF8 编码字节不会超过 length*3
// Base64 约 1-2倍
Maybe<size_t> StringBytes::StorageSize(Isolate* isolate,
                                       Local<Value> val,
                                       enum encoding encoding) {
  HandleScope scope(isolate);
  size_t data_size = 0;

  Local<String> str;
  if (!val->ToString(isolate->GetCurrentContext()).ToLocal(&str))
    return Nothing<size_t>();

  switch (encoding) {
    case ASCII:
    case LATIN1:
      data_size = str->Length();
      break;

    case BUFFER:
    case UTF8:
      // A single UCS2 codepoint never takes up more than 3 utf8 bytes.
      // It is an exercise for the caller to decide when a string is
      // long enough to justify calling Size() instead of StorageSize()
      data_size = 3 * str->Length();
      break;

    case UCS2:
      data_size = str->Length() * sizeof(uint16_t);
      break;

    case BASE64URL:
      // Fall through
    case BASE64:
      data_size = base64_decoded_size_fast(str->Length());
      break;

    case HEX:
      CHECK(str->Length() % 2 == 0 && "invalid hex string length");
      data_size = str->Length() / 2;
      break;

    default:
      CHECK(0 && "unknown encoding");
      break;
  }

  return Just(data_size);
}

}  // namespace pure

#include <cmath>
#include <iostream>

#include "pure.h"
#include "util.h"
#include "uv.h"
#include "v8.h"

namespace pure {

using v8::Isolate;
using v8::Local;
using v8::String;
using v8::Value;

template <typename T>
static void MakeUtf8String(Isolate* isolate,
                           Local<String> string,
                           MaybeStackBuffer<T>* target) {
  size_t storage;
  // TODO
  // if (!StringBytes::StorageSize(isolate, string, UTF8).To(&storage))
  //     return;
  storage += 1;
  target->AllocateSufficientStorage(storage);
  const int flags = String::NO_NULL_TERMINATION | String::REPLACE_INVALID_UTF8;
  const int length =
      string->WriteUtf8(isolate, target->out(), storage, nullptr, flags);
  target->SetLengthAndZeroTerminate(length);
}

template <typename T>
static void MakeUtf8String(Isolate* isolate,
                           Local<Value> value,
                           MaybeStackBuffer<T>* target) {
  Local<String> string;
  if (!value->ToString(isolate->GetCurrentContext()).ToLocal(&string)) return;

  MakeUtf8String(isolate, string, target);
}

Utf8Value::Utf8Value(Isolate* isolate, Local<Value> value) {
  if (value.IsEmpty()) return;

  MakeUtf8String(isolate, value, this);
}

Utf8Value::Utf8Value(Isolate* isolate, Local<String> value) {
  if (value.IsEmpty()) return;

  MakeUtf8String(isolate, value, this);
}

[[noreturn]] void Abort() {
  std::cout << ">> abort\n";

  std::fflush(stderr);
  std::abort();
}

[[noreturn]] void Assert(const AssertionInfo& info) {
  std::string location =
      std::string(info.file_line) + ":" + std::string(info.function);
  std::string message =
      "Assertion `" + std::string(info.message) + "' failed.\n";

  std::cout << message << ">>> assert\n";

  // TODO
  // OnFatalError(location.c_str(), message.c_str());
  std::abort();
}

void LowMemoryNotification() {
  if (per_process::v8_initialized) {
    auto isolate = Isolate::TryGetCurrent();
    if (isolate != nullptr) {
      isolate->LowMemoryNotification();
    }
  }
}
}  // namespace pure
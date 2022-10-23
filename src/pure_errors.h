#ifndef SRC_PURE_ERRORS_H_
#define SRC_PURE_ERRORS_H_

#include "v8.h"
#include "env.h"
#include "debug_utils-inl.h"

namespace pure {
namespace errors {
void PerIsolateMessageListener(v8::Local<v8::Message> message,
                               v8::Local<v8::Value> error);

void OnFatalError(const char* location, const char* message);

}  // namespace errors

#define ERRORS_WITH_CODE(V)                                                    \
  V(ERR_INVALID_OBJECT_DEFINE_PROPERTY, TypeError)                             \
  V(ERR_STRING_TOO_LONG, Error)

#define V(code, type)                                                          \
  template <typename... Args>                                                  \
  inline v8::Local<v8::Value> code(                                            \
      v8::Isolate* isolate, const char* format, Args&&... args) {              \
    std::string message = SPrintF(format, std::forward<Args>(args)...);        \
    v8::Local<v8::String> js_code = OneByteString(isolate, #code);             \
    v8::Local<v8::String> js_msg =                                             \
        OneByteString(isolate, message.c_str(), message.length());             \
    v8::Local<v8::Object> e = v8::Exception::type(js_msg)                      \
                                  ->ToObject(isolate->GetCurrentContext())     \
                                  .ToLocalChecked();                           \
    e->Set(isolate->GetCurrentContext(),                                       \
           OneByteString(isolate, "code"),                                     \
           js_code)                                                            \
        .Check();                                                              \
    return e;                                                                  \
  }                                                                            \
  template <typename... Args>                                                  \
  inline void THROW_##code(                                                    \
      v8::Isolate* isolate, const char* format, Args&&... args) {              \
    isolate->ThrowException(                                                   \
        code(isolate, format, std::forward<Args>(args)...));                   \
  }                                                                            \
  template <typename... Args>                                                  \
  inline void THROW_##code(                                                    \
      Environment* env, const char* format, Args&&... args) {                  \
    THROW_##code(env->isolate(), format, std::forward<Args>(args)...);         \
  }
ERRORS_WITH_CODE(V)
#undef V
}  // namespace pure

#endif  // SRC_PURE_ERRORS_H_

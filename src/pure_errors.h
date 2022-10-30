#ifndef SRC_PURE_ERRORS_H_
#define SRC_PURE_ERRORS_H_

#include "debug_utils-inl.h"
#include "env.h"
#include "v8.h"

namespace pure {
namespace errors {
void PerIsolateMessageListener(v8::Local<v8::Message> message,
                               v8::Local<v8::Value> error);

void OnFatalError(const char* location, const char* message);

class TryCatchScope : public v8::TryCatch {
 public:
  enum class CatchMode { kNormal, kFatal };

  explicit TryCatchScope(Environment* env, CatchMode mode = CatchMode::kNormal)
      : v8::TryCatch(env->isolate()), env_(env), mode_(mode) {}
  ~TryCatchScope();

  void* operator new(std::size_t count) = delete;
  void* operator new[](std::size_t count) = delete;
  TryCatchScope(TryCatchScope&) = delete;
  TryCatchScope(TryCatchScope&&) = delete;
  TryCatchScope operator=(TryCatchScope&) = delete;
  TryCatchScope operator=(TryCatchScope&&) = delete;

 private:
  Environment* env_;
  CatchMode mode_;
};

void TriggerUncaughtException(v8::Isolate* isolate,
                              const v8::TryCatch& try_catch);
void TriggerUncaughtException(v8::Isolate* isolate,
                              v8::Local<v8::Value> error,
                              v8::Local<v8::Message> message,
                              bool from_promise = false);

}  // namespace errors

#define ERRORS_WITH_CODE(V)                                                    \
  V(ERR_INVALID_OBJECT_DEFINE_PROPERTY, TypeError)                             \
  V(ERR_STRING_TOO_LONG, Error)                                                \
  V(ERR_INVALID_MODULE, Error)

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

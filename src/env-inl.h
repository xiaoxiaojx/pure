#ifndef SRC_ENV_INL_H_
#define SRC_ENV_INL_H_

#include "aliased_buffer.h"
#include "callback_queue-inl.h"
#include "env.h"
#include "util.h"
#include "uv.h"
#include "v8.h"

namespace pure {
// https://zhuanlan.zhihu.com/p/152055532
// 内联函数语法: inline要起作用,必须要与函数定义放在一起，而不是函数的声明
// 内联函数的作用:
// 当编译器处理调用内联函数的语句时，不会将该语句编译成函数调用的指令
// 而是直接将整个函数体的代码插人调用语句处，就像整个函数体在调用处被重写了一遍一样，在执行时是顺序执行，而不会进行跳

inline Environment* Environment::GetCurrent(v8::Isolate* isolate) {
  if (UNLIKELY(!isolate->InContext())) return nullptr;
  v8::HandleScope handle_scope(isolate);
  return GetCurrent(isolate->GetCurrentContext());
}

inline Environment* Environment::GetCurrent(v8::Local<v8::Context> context) {
  if (UNLIKELY(context.IsEmpty())) {
    return nullptr;
  }
  if (UNLIKELY(context->GetNumberOfEmbedderDataFields() <=
               ContextEmbedderIndex::kContextTag)) {
    return nullptr;
  }
  if (UNLIKELY(context->GetAlignedPointerFromEmbedderData(
                   ContextEmbedderIndex::kContextTag) !=
               Environment::kNodeContextTagPtr)) {
    return nullptr;
  }
  return static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(
      ContextEmbedderIndex::kEnvironment));
}

inline Environment* Environment::GetCurrent(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  return GetCurrent(info.GetIsolate()->GetCurrentContext());
}

template <typename T>
inline Environment* Environment::GetCurrent(
    const v8::PropertyCallbackInfo<T>& info) {
  return GetCurrent(info.GetIsolate()->GetCurrentContext());
}

#define V(PropertyName, TypeName)                                              \
  inline v8::Local<TypeName> Environment::PropertyName() const {               \
    return PersistentToLocal::Strong(PropertyName##_);                         \
  }                                                                            \
  inline void Environment::set_##PropertyName(v8::Local<TypeName> value) {     \
    PropertyName##_.Reset(isolate(), value);                                   \
  }
ENVIRONMENT_STRONG_PERSISTENT_TEMPLATES(V)
ENVIRONMENT_STRONG_PERSISTENT_VALUES(V)
#undef V

v8::Local<v8::Context> Environment::context() const {
  return PersistentToLocal::Strong(context_);
}

inline v8::Isolate* IsolateData::isolate() const {
  return isolate_;
}

inline uv_loop_t* IsolateData::event_loop() const {
  return event_loop_;
}

inline void Environment::AssignToContext(v8::Local<v8::Context> context,
                                         const ContextInfo& info) {
  context->SetAlignedPointerInEmbedderData(ContextEmbedderIndex::kEnvironment,
                                           this);
  // Used by Environment::GetCurrent to know that we are on a node context.
  context->SetAlignedPointerInEmbedderData(ContextEmbedderIndex::kContextTag,
                                           Environment::kNodeContextTagPtr);
}

inline IsolateData* Environment::isolate_data() const {
  return isolate_data_;
}

inline uv_loop_t* Environment::event_loop() const {
  return isolate_data()->event_loop();
}

inline bool Environment::is_stopping() const {
  return is_stopping_.load();
}

inline void Environment::set_stopping(bool value) {
  is_stopping_.store(value);
}

inline v8::Isolate* Environment::isolate() const {
  return isolate_;
}

inline bool Environment::can_call_into_js() const {
  return can_call_into_js_ && !is_stopping();
}

inline void Environment::set_can_call_into_js(bool can_call_into_js) {
  can_call_into_js_ = can_call_into_js;
}

inline bool Environment::is_main_thread() const {
  // TODO 暂时不支持线程
  return true;
}

inline bool Environment::abort_on_uncaught_exception() const {
  return options_->abort_on_uncaught_exception;
}

inline bool Environment::has_run_bootstrapping_code() const {
  return has_run_bootstrapping_code_;
}

inline uv_check_t* Environment::immediate_check_handle() {
  return &immediate_check_handle_;
}

inline uv_timer_t* Environment::timer_handle() {
  return &timer_handle_;
}

inline ImmediateInfo* Environment::immediate_info() {
  return &immediate_info_;
}

inline std::shared_ptr<EnvironmentOptions> Environment::options() {
  return options_;
}

inline std::shared_ptr<KVStore> Environment::env_vars() {
  return env_vars_;
}

inline void Environment::set_env_vars(std::shared_ptr<KVStore> env_vars) {
  env_vars_ = env_vars;
}

inline void Environment::set_abort_on_uncaught_exception(bool value) {
  options_->abort_on_uncaught_exception = value;
}

inline Environment* Environment::from_immediate_check_handle(
    uv_check_t* handle) {
  return ContainerOf(&Environment::immediate_check_handle_, handle);
}

inline std::vector<std::string> Environment::argv() {
  return argv_;
}

inline uv_idle_t* Environment::immediate_idle_handle() {
  return &immediate_idle_handle_;
}

inline void Environment::DoneBootstrapping() {
  has_run_bootstrapping_code_ = true;
  // This adjusts the return value of base_object_created_after_bootstrap() so
  // that tests that check the count do not have to account for internally
  // created BaseObjects.
  // base_object_created_by_bootstrap_ = base_object_count_;
}

inline v8::Local<v8::FunctionTemplate> Environment::NewFunctionTemplate(
    v8::FunctionCallback callback,
    v8::Local<v8::Signature> signature,
    v8::ConstructorBehavior behavior,
    v8::SideEffectType side_effect_type,
    const v8::CFunction* c_function) {
  return v8::FunctionTemplate::New(isolate(),
                                   callback,
                                   v8::Local<v8::Value>(),
                                   signature,
                                   0,
                                   behavior,
                                   side_effect_type,
                                   c_function);
}

template <typename Fn>
void Environment::SetImmediateThreadsafe(Fn&& cb, CallbackFlags::Flags flags) {
  auto callback =
      native_immediates_threadsafe_.CreateCallback(std::move(cb), flags);

  {
    Mutex::ScopedLock lock(native_immediates_threadsafe_mutex_);
    native_immediates_threadsafe_.Push(std::move(callback));
    if (task_queues_async_initialized_) uv_async_send(&task_queues_async_);
  }
}

template <typename Fn>
void Environment::RequestInterrupt(Fn&& cb) {
  auto callback = native_immediates_interrupts_.CreateCallback(
      std::move(cb), CallbackFlags::kRefed);
  {
    Mutex::ScopedLock lock(native_immediates_threadsafe_mutex_);
    native_immediates_interrupts_.Push(std::move(callback));
    if (task_queues_async_initialized_) uv_async_send(&task_queues_async_);
  }
  RequestInterruptFromV8();
}

void Environment::set_process_exit_handler(
    std::function<void(Environment*, int)>&& handler) {
  process_exit_handler_ = std::move(handler);
}

inline void Environment::SetMethod(v8::Local<v8::Object> that,
                                   const char* name,
                                   v8::FunctionCallback callback) {
  v8::Local<v8::Context> context = isolate()->GetCurrentContext();
  v8::Local<v8::Function> function =
      NewFunctionTemplate(callback,
                          v8::Local<v8::Signature>(),
                          v8::ConstructorBehavior::kThrow,
                          v8::SideEffectType::kHasSideEffect)
          ->GetFunction(context)
          .ToLocalChecked();
  // kInternalized strings are created in the old space.
  const v8::NewStringType type = v8::NewStringType::kInternalized;
  v8::Local<v8::String> name_string =
      v8::String::NewFromUtf8(isolate(), name, type).ToLocalChecked();
  that->Set(context, name_string, function).Check();
  function->SetName(name_string);  // NODE_SET_METHOD() compatibility.
}

inline AliasedUint32Array& ImmediateInfo::fields() {
  return fields_;
}

inline uint32_t ImmediateInfo::count() const {
  return fields_[kCount];
}

inline uint32_t ImmediateInfo::ref_count() const {
  return fields_[kRefCount];
}

inline bool ImmediateInfo::has_outstanding() const {
  return fields_[kHasOutstanding] == 1;
}

inline void ImmediateInfo::ref_count_inc(uint32_t increment) {
  fields_[kRefCount] += increment;
}

inline void ImmediateInfo::ref_count_dec(uint32_t decrement) {
  fields_[kRefCount] -= decrement;
}

inline std::shared_ptr<PerIsolateOptions> IsolateData::options() {
  return options_;
}

inline void IsolateData::set_options(
    std::shared_ptr<PerIsolateOptions> options) {
  options_ = std::move(options);
}
}  // namespace pure

#endif  // SRC_ENV_INL_H_

#ifndef SRC_ENV_H_
#define SRC_ENV_H_

#include <vector>
#include <list>

#include "aliased_buffer.h"
#include "callback_queue.h"
#include "pure.h"
#include "pure_mutex.h"
#include "pure_options.h"
#include "util.h"
#include "uv.h"
#include "v8.h"

namespace pure {

#define ENVIRONMENT_STRONG_PERSISTENT_VALUES(V)                                \
  V(immediate_callback_function, v8::Function)                                 \
  V(promise_reject_callback, v8::Function)

#define ENVIRONMENT_STRONG_PERSISTENT_TEMPLATES(V)                             \
  V(binding_data_ctor_template, v8::FunctionTemplate)

#define PER_ISOLATE_STRING_PROPERTIES(V) V(exit_code_string, "exitCode")

using v8::Context;
using v8::Isolate;
using v8::Local;

using v8::Isolate;
using v8::Local;
using v8::Maybe;
using v8::MaybeLocal;
using v8::Object;
using v8::Value;

Local<Context> NewContext(Isolate*);

class PerIsolateOptions;
class IsolateData;

struct EnvSerializeInfo {
  // TODO
};

class ImmediateInfo {
 public:
  inline AliasedUint32Array& fields();
  inline uint32_t count() const;
  inline uint32_t ref_count() const;
  inline bool has_outstanding() const;
  inline void ref_count_inc(uint32_t increment);
  inline void ref_count_dec(uint32_t decrement);

  ImmediateInfo(const ImmediateInfo&) = delete;
  ImmediateInfo& operator=(const ImmediateInfo&) = delete;
  ImmediateInfo(ImmediateInfo&&) = delete;
  ImmediateInfo& operator=(ImmediateInfo&&) = delete;
  ~ImmediateInfo() = default;
  //   ImmediateInfo(v8::Isolate* isolate);

 private:
  friend class Environment;  // So we can call the constructor.
  explicit ImmediateInfo(v8::Isolate* isolate);

  enum Fields { kCount, kRefCount, kHasOutstanding, kFieldsCount };

  AliasedUint32Array fields_;
};

class KVStore {
 public:
  KVStore() = default;
  virtual ~KVStore() = default;
  KVStore(const KVStore&) = delete;
  KVStore& operator=(const KVStore&) = delete;
  KVStore(KVStore&&) = delete;
  KVStore& operator=(KVStore&&) = delete;

  virtual v8::MaybeLocal<v8::String> Get(v8::Isolate* isolate,
                                         v8::Local<v8::String> key) const = 0;
  virtual v8::Maybe<std::string> Get(const char* key) const = 0;
  virtual void Set(v8::Isolate* isolate,
                   v8::Local<v8::String> key,
                   v8::Local<v8::String> value) = 0;
  virtual int32_t Query(v8::Isolate* isolate,
                        v8::Local<v8::String> key) const = 0;
  virtual int32_t Query(const char* key) const = 0;
  virtual void Delete(v8::Isolate* isolate, v8::Local<v8::String> key) = 0;
  virtual v8::Local<v8::Array> Enumerate(v8::Isolate* isolate) const = 0;

  virtual std::shared_ptr<KVStore> Clone(v8::Isolate* isolate) const;
  virtual v8::Maybe<bool> AssignFromObject(v8::Local<v8::Context> context,
                                           v8::Local<v8::Object> entries);

  static std::shared_ptr<KVStore> CreateMapKVStore();
};

class Environment {
 public:
  // Copy Constructor
  Environment(const Environment&) = delete;
  // Copy Assignment operator
  Environment& operator=(const Environment&) = delete;
  // Move Constructor
  Environment(Environment&&) = delete;
  // Move Assignment operator
  Environment& operator=(Environment&&) = delete;

  // C++右值引用 https://zhuanlan.zhihu.com/p/335994370
  // C++ 语言中，函数的参数和返回值的传递方式有三种：值传递、指针传递和引用传递
  // https://www.runoob.com/w3cnote/cpp-difference-between-pointers-and-references.html

  // =default 和 =delete https://zhuanlan.zhihu.com/p/173231137
  // = delete 禁止使用编译器默认生成的函数
  // = defaule
  // 在程序员重载了自己上面提到的C++编译器默认生成的函数之后，C++编译器将不在生成这些函数的默认形式。但是C++允许我们使用=default来要求编译器生成一个默认函数

  static inline Environment* GetCurrent(v8::Isolate* isolate);
  static inline Environment* GetCurrent(v8::Local<v8::Context> context);
  static inline Environment* GetCurrent(
      const v8::FunctionCallbackInfo<v8::Value>& info);

  template <typename T>
  static inline Environment* GetCurrent(
      const v8::PropertyCallbackInfo<T>& info);

  inline v8::Local<v8::Context> context() const;

  // Create an Environment without initializing a main Context. Use
  // InitializeMainContext() to initialize a main context for it.
  Environment(
      IsolateData* isolate_data,
      v8::Isolate* isolate,
      // vector https://blog.csdn.net/u014779536/article/details/111239643
      // vector 是表示可以改变大小的数组的序列容器。
      const std::vector<std::string>& args,
      const std::vector<std::string>& exec_args,
      EnvironmentFlags::Flags flags);
  void InitializeMainContext(v8::Local<v8::Context> context,
                             const EnvSerializeInfo* env_info);
  // Create an Environment and initialize the provided main context for it.
  Environment(IsolateData* isolate_data,
              v8::Local<v8::Context> context,
              const std::vector<std::string>& args,
              const std::vector<std::string>& exec_args,
              EnvironmentFlags::Flags flags);
  ~Environment();

  void CleanupHandles();
  void RunCleanup();
  void Exit(int code);
  void ExitEnv();

  void AtExit(void (*cb)(void* arg), void* arg);
  void RunAtExitCallbacks();
  void RequestInterruptFromV8();
  void RunAndClearInterrupts();

  v8::MaybeLocal<v8::Value> RunBootstrapping();
  inline std::vector<std::string> argv();

  inline v8::Isolate* isolate() const;

  inline IsolateData* isolate_data() const;
  inline uv_loop_t* event_loop() const;
  std::atomic_bool is_stopping_{false};

  inline bool is_stopping() const;
  inline void set_stopping(bool value);
  inline bool is_main_thread() const;

  inline bool can_call_into_js() const;
  inline void set_can_call_into_js(bool can_call_into_js);

  inline bool abort_on_uncaught_exception() const;

  inline uv_check_t* immediate_check_handle();
  inline uv_timer_t* timer_handle();
  inline uv_idle_t* immediate_idle_handle();
  void RunAndClearNativeImmediates(bool only_refed = false);
  void ToggleImmediateRef(bool ref);
  inline void set_process_exit_handler(
      std::function<void(Environment*, int)>&& handler);
  bool started_cleanup_ = false;

  inline bool has_run_bootstrapping_code() const;
  inline void DoneBootstrapping();
  // v8::MaybeLocal<v8::Value> BootstrapNode();
  Maybe<bool> BootstrapPure();
  inline ImmediateInfo* immediate_info();
  inline std::shared_ptr<EnvironmentOptions> options();
  inline std::shared_ptr<KVStore> env_vars();
  inline void set_env_vars(std::shared_ptr<KVStore> env_vars);

  void InitializeLibuv();

  inline bool EmitProcessEnvWarning() {
    bool current_value = emit_env_nonstring_warning_;
    emit_env_nonstring_warning_ = false;
    return current_value;
  }

  template <typename Fn>
  // This behaves like SetImmediate() but can be called from any thread.
  inline void SetImmediateThreadsafe(Fn&& cb, CallbackFlags::Flags flags);
  static inline Environment* from_immediate_check_handle(uv_check_t* handle);

  struct ContextInfo {
    explicit ContextInfo(const std::string& name) : name(name) {}
    const std::string name;
    std::string origin;
    bool is_default = false;
  };

  void CreateProperties();

  template <typename Fn>
  inline void RequestInterrupt(Fn&& cb);

  inline void AssignToContext(v8::Local<v8::Context> context,
                              const ContextInfo& info);

  inline v8::Local<v8::FunctionTemplate> NewFunctionTemplate(
      v8::FunctionCallback callback,
      v8::Local<v8::Signature> signature = v8::Local<v8::Signature>(),
      v8::ConstructorBehavior behavior = v8::ConstructorBehavior::kAllow,
      v8::SideEffectType side_effect = v8::SideEffectType::kHasSideEffect,
      const v8::CFunction* c_function = nullptr);

  // Convenience methods for NewFunctionTemplate().
  inline void SetMethod(v8::Local<v8::Object> that,
                        const char* name,
                        v8::FunctionCallback callback);

#define V(PropertyName, TypeName)                                              \
  inline v8::Local<TypeName> PropertyName() const;                             \
  inline void set_##PropertyName(v8::Local<TypeName> value);
  ENVIRONMENT_STRONG_PERSISTENT_VALUES(V)
  ENVIRONMENT_STRONG_PERSISTENT_TEMPLATES(V)
#undef V

 private:
#define V(PropertyName, TypeName) v8::Global<TypeName> PropertyName##_;
  ENVIRONMENT_STRONG_PERSISTENT_VALUES(V)
  ENVIRONMENT_STRONG_PERSISTENT_TEMPLATES(V)
#undef V
  v8::Global<v8::Context> context_;
  std::shared_ptr<KVStore> env_vars_;

  static void* const kNodeContextTagPtr;
  static int const kNodeContextTag;

  v8::Isolate* const isolate_;
  IsolateData* const isolate_data_;
  uv_timer_t timer_handle_;
  uv_check_t immediate_check_handle_;
  uv_idle_t immediate_idle_handle_;
  uv_prepare_t idle_prepare_handle_;
  uv_check_t idle_check_handle_;
  uv_async_t task_queues_async_;
  int64_t task_queues_async_refs_ = 0;
  bool has_run_bootstrapping_code_ = false;
  int64_t base_object_count_ = 0;

  ImmediateInfo immediate_info_;
  // TickInfo tick_info_;
  const uint64_t timer_base_;
  // std::shared_ptr<KVStore> env_vars_;
  bool printed_error_ = false;
  bool trace_sync_io_ = false;
  bool emit_env_nonstring_warning_ = true;
  bool emit_err_name_warning_ = true;
  bool emit_filehandle_warning_ = true;
  bool source_maps_enabled_ = false;
  std::atomic_bool can_call_into_js_{true};

  size_t async_callback_scope_depth_ = 0;
  std::vector<double> destroy_async_id_list_;

  std::vector<std::string> exec_argv_;
  std::vector<std::string> argv_;
  std::string exec_path_;
  std::shared_ptr<EnvironmentOptions> options_;

  uint64_t environment_start_time_;

  uint64_t flags_;
  uint64_t thread_id_;
  std::atomic<Environment**> interrupt_data_{nullptr};

  typedef CallbackQueue<void, Environment*> NativeImmediateQueue;
  NativeImmediateQueue native_immediates_;

  Mutex native_immediates_threadsafe_mutex_;
  NativeImmediateQueue native_immediates_threadsafe_;
  NativeImmediateQueue native_immediates_interrupts_;


  bool task_queues_async_initialized_ = false;
  std::function<void(Environment*, int)> process_exit_handler_{
      DefaultProcessExitHandler};

  struct ExitCallback {
    void (*cb_)(void* arg);
    void* arg_;
  };

  std::list<ExitCallback> at_exit_functions_;

#define VP(PropertyName, StringValue) V(v8::Private, PropertyName)
#define VY(PropertyName, StringValue) V(v8::Symbol, PropertyName)
#define VS(PropertyName, StringValue) V(v8::String, PropertyName)
#define V(TypeName, PropertyName)                                              \
  inline v8::Local<TypeName> PropertyName() const;
  PER_ISOLATE_STRING_PROPERTIES(VS)
#undef V
#undef VS
#undef VY
#undef VP

  static void CheckImmediate(uv_check_t* handle);

  // AliasedUint32Array should_abort_on_uncaught_toggle_;
  // int should_not_abort_scope_counter_ = 0;
};

class PURE_EXTERN_PRIVATE IsolateData {
 public:
  IsolateData(v8::Isolate* isolate,
              uv_loop_t* event_loop,
              v8::Platform* platform);

  inline uv_loop_t* event_loop() const;
  inline v8::Platform* platform() const;

  IsolateData(const IsolateData&) = delete;
  IsolateData& operator=(const IsolateData&) = delete;
  IsolateData(IsolateData&&) = delete;
  IsolateData& operator=(IsolateData&&) = delete;

  inline v8::Isolate* isolate() const;

 private:
  v8::Isolate* const isolate_;
  uv_loop_t* const event_loop_;
  v8::Platform* platform_;
  std::shared_ptr<PerIsolateOptions> options_;

  void CreateProperties();
};

v8::MaybeLocal<v8::Value> ExecuteBootstrapper(
    Environment* env,
    const char* id,
    std::vector<v8::Local<v8::String>>* parameters,
    std::vector<v8::Local<v8::Value>>* arguments);
}  // namespace pure

#endif  // SRC_ENV_H_
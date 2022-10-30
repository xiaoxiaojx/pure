#include "env.h"
#include "env-inl.h"

#include "util-inl.h"
#include "util.h"
#include "uv.h"
#include "v8.h"

#include <iostream>
#include <memory>

#include "pure.h"
#include "pure_options.h"

#include "pure_binding.h"
#include "pure_errors.h"
#include "pure_native_module.h"

namespace pure {

using v8::Context;
using v8::EscapableHandleScope;
using v8::Function;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::Isolate;
using v8::Just;
using v8::Local;
using v8::Maybe;
using v8::MaybeLocal;
using v8::Nothing;
using v8::Object;
using v8::Script;
using v8::SealHandleScope;
using v8::String;
using v8::TryCatch;
using v8::Value;

IsolateData::IsolateData(Isolate* isolate,
                         uv_loop_t* event_loop,
                         v8::Platform* platform)
    : isolate_(isolate), event_loop_(event_loop), platform_(platform) {
  options_.reset(
      new PerIsolateOptions(*(per_process::cli_options->per_isolate)));

  CreateProperties();
}

// TODO 实现 setImmediate
// fields_ 可用于 js 直接操作, 见 aliased_buffer.h
// ref => immediateInfo[kRefCount]++
// unref => --immediateInfo[kRefCount]
ImmediateInfo::ImmediateInfo(Isolate* isolate)
    : fields_(isolate, kFieldsCount, nullptr) {}

void IsolateData::CreateProperties() {
  // TODO
}

// 通过 SetAlignedPointerInEmbedderData 设置一个私有属性值,
// 这是很通用的一个方法, 在 Js 对象上挂载一个 C 对象
int const Environment::kNodeContextTag = 0x6e6f64;
void* const Environment::kNodeContextTagPtr =
    // const_cast 的例子
    // ❌ const int constant = 21;
    // int* modifier = &constant
    // Error: invalid conversion from 'const int*' to 'int*'

    // ✅ const int constant = 21;
    // const int* const_p = &constant;
    // int* modifier = const_cast<int*>(const_p);
    // *modifier = 7;

    // ✅ const int constant = 21;
    // int* modifier = (int*)(&constant);

    const_cast<void*>(static_cast<const void*>(&Environment::kNodeContextTag));

std::string GetExecPath(const std::vector<std::string>& argv) {
  char exec_path_buf[2 * PATH_MAX];
  size_t exec_path_len = sizeof(exec_path_buf);
  std::string exec_path;
  if (uv_exepath(exec_path_buf, &exec_path_len) == 0) {
    exec_path = std::string(exec_path_buf, exec_path_len);
  } else {
    exec_path = argv[0];
  }

  return exec_path;
}

void Environment::InitializeMainContext(Local<Context> context,
                                        const EnvSerializeInfo* env_info) {
  context_.Reset(context->GetIsolate(), context);
  AssignToContext(context, ContextInfo(""));
  CreateProperties();
}

void Environment::CreateProperties() {
  HandleScope handle_scope(isolate_);
  Local<Context> ctx = context();

  {
    // Context::Scope 用于管理 context, 用于控制当前 js 运行的上下文,
    // 比如 https://github.com/mcollina/worker 的实现, 在一个线程中用的新的
    // context 与 isolate

    // Context: JavaScript 允许多个全局对象和内置 JavaScript
    // 对象集（如 Object 或 Array 函数）在同一个堆内共存。 Node.js 通过 vm
    // 模块公开了这种能力。
    Context::Scope context_scope(ctx);
    Local<FunctionTemplate> templ = FunctionTemplate::New(isolate());
    set_binding_data_ctor_template(templ);
  }
}

Environment::Environment(IsolateData* isolate_data,
                         Isolate* isolate,
                         const std::vector<std::string>& args,
                         const std::vector<std::string>& exec_args,
                         EnvironmentFlags::Flags flags)
    : isolate_(isolate),
      isolate_data_(isolate_data),
      immediate_info_(isolate),
      //   tick_info_(isolate, MAYBE_FIELD_PTR(env_info, tick_info)),
      timer_base_(uv_now(isolate_data->event_loop())),
      exec_argv_(exec_args),
      argv_(args),
      exec_path_(GetExecPath(args)),
      environment_start_time_(PERFORMANCE_NOW()),
      flags_(flags) {
  HandleScope handle_scope(isolate);

  // TODO 暂时只支持 kDefaultFlags
  if (flags_ & EnvironmentFlags::kDefaultFlags) {
    flags_ = flags_ | EnvironmentFlags::kOwnsProcessState |
             EnvironmentFlags::kOwnsInspector;
  }

  set_env_vars(per_process::system_environment);

  options_ =
      std::make_shared<EnvironmentOptions>(*isolate_data->options()->per_env);

  if (!(flags_ & EnvironmentFlags::kOwnsProcessState)) {
    set_abort_on_uncaught_exception(false);
  }
}

Environment::Environment(IsolateData* isolate_data,
                         Local<Context> context,
                         const std::vector<std::string>& args,
                         const std::vector<std::string>& exec_args,
                         EnvironmentFlags::Flags flags)
    : Environment(isolate_data, context->GetIsolate(), args, exec_args, flags) {
  InitializeMainContext(context, nullptr);
}

Environment::~Environment() {
  if (Environment** interrupt_data = interrupt_data_.load()) {
    *interrupt_data = nullptr;

    // 用于恢复 DisallowJavascriptExecutionScope, demo 见 demo/v8-api-test.cc
    Isolate::AllowJavascriptExecutionScope allow_js_here(isolate());
    HandleScope handle_scope(isolate());
    // 当前作用域 js 运行出错, 信息将挂载在 try_catch 上
    TryCatch try_catch(isolate());
    Context::Scope context_scope(context());

    Local<Script> script;
    if (Script::Compile(context(), String::Empty(isolate())).ToLocal(&script))
      USE(script->Run(context()));

    DCHECK(consistency_check);
  }

  CHECK(is_stopping());

  // if (options_->heap_snapshot_near_heap_limit > heap_limit_snapshot_taken_) {
  //   isolate_->RemoveNearHeapLimitCallback(Environment::NearHeapLimitCallback,
  //                                         0);
  // }

  HandleScope handle_scope(isolate());

  context()->SetAlignedPointerInEmbedderData(ContextEmbedderIndex::kEnvironment,
                                             nullptr);

  // TODO 支持 C++ 模块
  // if (!is_main_thread()) {
  //   // Dereference all addons that were loaded into this environment.
  //   for (binding::DLib& addon : loaded_addons_) {
  //     addon.Close();
  //   }
  // }

  CHECK_EQ(base_object_count_, 0);
}

void Environment::CleanupHandles() {
  {
    Mutex::ScopedLock lock(native_immediates_threadsafe_mutex_);
    task_queues_async_initialized_ = false;
  }

  // 当前作用域运行 js 代码将会抛错, demo 见 demo/v8-api-test.cc
  Isolate::DisallowJavascriptExecutionScope disallow_js(
      isolate(), Isolate::DisallowJavascriptExecutionScope::THROW_ON_FAILURE);

  RunAndClearNativeImmediates(true);

  { uv_run(event_loop(), UV_RUN_ONCE); }
}

void Environment::Exit(int exit_code) {
  fprintf(stderr, "WARNING: Exited the environment with code %d\n", exit_code);
  process_exit_handler_(this, exit_code);
}

void Environment::AtExit(void (*cb)(void* arg), void* arg) {
  at_exit_functions_.push_front(ExitCallback{cb, arg});
}

void Environment::RunCleanup() {
  started_cleanup_ = true;
  // TODO 支持 C++ 模块
  // bindings_.clear();
  CleanupHandles();
}

void Environment::RunAtExitCallbacks() {
  for (ExitCallback at_exit : at_exit_functions_) {
    at_exit.cb_(at_exit.arg_);
  }
  at_exit_functions_.clear();
}

void RunAtExit(Environment* env) {
  env->RunAtExitCallbacks();
}

void Environment::RunAndClearInterrupts() {
  while (native_immediates_interrupts_.size() > 0) {
    NativeImmediateQueue queue;
    {
      Mutex::ScopedLock lock(native_immediates_threadsafe_mutex_);
      queue.ConcatMove(std::move(native_immediates_interrupts_));
    }
    SealHandleScope seal_handle_scope(isolate());

    while (auto head = queue.Shift()) head->Call(this);
  }
}

// 调用 v8 的 RequestInterrupt 函数, 强制执行一次传入的 callback,
// 防止 Js 长时间运行一个循环或者一个长任务
void Environment::RequestInterruptFromV8() {
  Environment** interrupt_data = new Environment*(this);
  Environment** dummy = nullptr;

  // 如果 interrupt_data_ == dummy, 则 compare_exchange_strong 返回 true,
  // 然后把 interrupt_data_ 赋值为 interrupt_data
  if (!interrupt_data_.compare_exchange_strong(dummy, interrupt_data)) {
    delete interrupt_data;
    return;
  }

  // 如果 interrupt_data_ 是 nullptr
  isolate()->RequestInterrupt(
      [](Isolate* isolate, void* data) {
        std::unique_ptr<Environment*> env_ptr{static_cast<Environment**>(data)};
        Environment* env = *env_ptr;

        // ~Environment
        if (env == nullptr) {
          return;
        }

        env->interrupt_data_.store(nullptr);

        // 需要被执行的代码
        env->RunAndClearInterrupts();
      },
      interrupt_data);
}

void DefaultProcessExitHandler(Environment* env, int exit_code) {
  env->set_can_call_into_js(false);
  // 释放 libuv 持有的任何全局状态。 Libuv
  // 通常会在卸载时自动执行此操作，但可以指示它手动执行清理。调用
  // uv_library_shutdown() 后不要调用 libuv 函数
  uv_library_shutdown();
  exit(exit_code);
}

void FreeEnvironment(Environment* env) {
  Isolate* isolate = env->isolate();
  Isolate::DisallowJavascriptExecutionScope disallow_js(
      isolate, Isolate::DisallowJavascriptExecutionScope::THROW_ON_FAILURE);
  {
    HandleScope handle_scope(isolate);
    Context::Scope context_scope(env->context());
    SealHandleScope seal_handle_scope(isolate);

    env->set_stopping(true);
    env->RunCleanup();
    RunAtExit(env);
  }

  delete env;
}

bool ShouldAbortOnUncaughtException(Isolate* isolate) {
  SealHandleScope scope(isolate);
  Environment* env = Environment::GetCurrent(isolate);
  return env != nullptr && (env->is_main_thread() || !env->is_stopping()) &&
         env->abort_on_uncaught_exception();
}

Maybe<bool> InitializeContext(Local<Context> context) {
  return Just(true);
}

Local<Context> NewContext(Isolate* isolate) {
  auto context = Context::New(isolate, nullptr);
  if (context.IsEmpty()) {
    std::cout << "NewContext context.IsEmpty()" << std::endl;
    return context;
  }

  if (InitializeContext(context).IsNothing()) {
    return Local<Context>();
  }

  return context;
}

Maybe<bool> Environment::BootstrapPure() {
  HandleScope scope(isolate_);
  // 好吧, 我只想保持和 JavaScript 中相同的用法
  // 虽然在 C 中完全没必要这样绕个弯子来引模块
  // GetInternalBinding(this, "addon_console") = require("addon_console")
  Local<Object> __pure_stdout =
      binding::GetInternalBinding(this, "addon_console").ToLocalChecked();
  Local<Object> global = context()->Global();
  global->Set(context(),
              FIXED_ONE_BYTE_STRING(isolate_, "__pure_stdout"),
              __pure_stdout);
  global->Set(context(), FIXED_ONE_BYTE_STRING(isolate_, "global"), global)
      .Check();
  return Just(true);
}

MaybeLocal<Value> Environment::RunBootstrapping() {
  HandleScope scope(isolate_);
  std::vector<Local<String>> loaders_params = {};
  std::vector<Local<Value>> loaders_args = {};

  // 注入 addon
  BootstrapPure();
}

MaybeLocal<Value> ExecuteBootstrapper(Environment* env,
                                      const char* id,
                                      std::vector<Local<String>>* parameters,
                                      std::vector<Local<Value>>* arguments) {
  EscapableHandleScope scope(env->isolate());
  MaybeLocal<Function> maybe_fn =
      native_module::NativeModuleLoader::GetInstance()->LookupAndCompile(
          env->context(), id, parameters, env);

  Local<Function> fn;
  if (!maybe_fn.ToLocal(&fn)) {
    return MaybeLocal<Value>();
  }

  MaybeLocal<Value> result = fn->Call(env->context(),
                                      Undefined(env->isolate()),
                                      arguments->size(),
                                      arguments->data());

  return scope.EscapeMaybe(result);
}

static MaybeLocal<Value> StartExecution(Environment* env,
                                        const char* main_script_id) {
  EscapableHandleScope scope(env->isolate());
  CHECK_NOT_NULL(main_script_id);

  std::vector<Local<String>> parameters = {};

  std::vector<Local<Value>> arguments = {};

  return scope.EscapeMaybe(
      ExecuteBootstrapper(env, main_script_id, &parameters, &arguments));
}

MaybeLocal<Value> StartExecution(Environment* env, StartExecutionCallback cb) {
  InternalCallbackScope callback_scope(env, Object::New(env->isolate()));

  if (cb != nullptr) {
    EscapableHandleScope scope(env->isolate());

    if (StartExecution(env, "internal/bootstrap/environment").IsEmpty())
      return {};

    StartExecutionCallbackInfo info = {};

    return scope.EscapeMaybe(cb(info));
  }

  StartExecution(env, "pure:console");

  return StartExecution(env, env->argv().at(1).c_str());
}

MaybeLocal<Value> LoadEnvironment(Environment* env, StartExecutionCallback cb) {
  env->InitializeLibuv();
  //   env->InitializeDiagnostics();

  return StartExecution(env, cb);
}

void Environment::InitializeLibuv() {
  HandleScope handle_scope(isolate());
  Context::Scope context_scope(context());

  CHECK_EQ(0, uv_timer_init(event_loop(), timer_handle()));
  uv_unref(reinterpret_cast<uv_handle_t*>(timer_handle()));

  CHECK_EQ(0, uv_check_init(event_loop(), immediate_check_handle()));
  uv_unref(reinterpret_cast<uv_handle_t*>(immediate_check_handle()));

  CHECK_EQ(0, uv_idle_init(event_loop(), immediate_idle_handle()));

  CHECK_EQ(0, uv_check_start(immediate_check_handle(), CheckImmediate));

  CHECK_EQ(0, uv_prepare_init(event_loop(), &idle_prepare_handle_));
  CHECK_EQ(0, uv_check_init(event_loop(), &idle_check_handle_));

  CHECK_EQ(
      0,
      uv_async_init(event_loop(), &task_queues_async_, [](uv_async_t* async) {
        Environment* env = ContainerOf(&Environment::task_queues_async_, async);
        HandleScope handle_scope(env->isolate());
        Context::Scope context_scope(env->context());
        env->RunAndClearNativeImmediates();
      }));
  uv_unref(reinterpret_cast<uv_handle_t*>(&idle_prepare_handle_));
  uv_unref(reinterpret_cast<uv_handle_t*>(&idle_check_handle_));
  uv_unref(reinterpret_cast<uv_handle_t*>(&task_queues_async_));

  {
    Mutex::ScopedLock lock(native_immediates_threadsafe_mutex_);
    task_queues_async_initialized_ = true;
    if (native_immediates_threadsafe_.size() > 0 ||
        native_immediates_interrupts_.size() > 0) {
      uv_async_send(&task_queues_async_);
    }
  }

  //   RegisterHandleCleanups();

  //   StartProfilerIdleNotifier();
}

void Environment::RunAndClearNativeImmediates(bool only_refed) {
  HandleScope handle_scope(isolate_);
  InternalCallbackScope cb_scope(this, Object::New(isolate_));

  size_t ref_count = 0;

  // Handle interrupts first. These functions are not allowed to throw
  // exceptions, so we do not need to handle that.
  RunAndClearInterrupts();

  auto drain_list = [&](NativeImmediateQueue* queue) {
    errors::TryCatchScope try_catch(this);
    SealHandleScope seal_handle_scope(isolate());
    while (auto head = queue->Shift()) {
      bool is_refed = head->flags() & CallbackFlags::kRefed;
      if (is_refed) ref_count++;

      if (is_refed || !only_refed) head->Call(this);

      head.reset();  // Destroy now so that this is also observed by

      if (UNLIKELY(try_catch.HasCaught())) {
        if (!try_catch.HasTerminated() && can_call_into_js())
          errors::TriggerUncaughtException(isolate(), try_catch);

        return true;
      }
    }
    return false;
  };
  while (drain_list(&native_immediates_)) {
  }

  immediate_info()->ref_count_dec(ref_count);

  if (immediate_info()->ref_count() == 0) ToggleImmediateRef(false);

  NativeImmediateQueue threadsafe_immediates;
  if (native_immediates_threadsafe_.size() > 0) {
    Mutex::ScopedLock lock(native_immediates_threadsafe_mutex_);
    threadsafe_immediates.ConcatMove(std::move(native_immediates_threadsafe_));
  }
  while (drain_list(&threadsafe_immediates)) {
  }
}

void Environment::CheckImmediate(uv_check_t* handle) {
  Environment* env = Environment::from_immediate_check_handle(handle);
  HandleScope scope(env->isolate());
  Context::Scope context_scope(env->context());

  env->RunAndClearNativeImmediates();

  if (env->immediate_info()->count() == 0 || !env->can_call_into_js()) return;

  Local<Object> recv;

  do {
    MakeCallback(
        env->isolate(), recv, env->immediate_callback_function(), 0, nullptr)
        .ToLocalChecked();
  } while (env->immediate_info()->has_outstanding() && env->can_call_into_js());

  if (env->immediate_info()->ref_count() == 0) env->ToggleImmediateRef(false);
}

void Environment::ToggleImmediateRef(bool ref) {
  if (started_cleanup_) return;

  if (ref) {
    uv_idle_start(immediate_idle_handle(), [](uv_idle_t*) {});
  } else {
    uv_idle_stop(immediate_idle_handle());
  }
}

void Environment::ExitEnv() {
  set_can_call_into_js(false);
  set_stopping(true);
  // 使得 JavaScript 执行终止, 可以通过调用 isolate_->CancelTerminateExecution()
  // 取消该操作
  isolate_->TerminateExecution();
  SetImmediateThreadsafe([](Environment* env) { uv_stop(env->event_loop()); },
                         CallbackFlags::kRefed);
}
}  // namespace pure
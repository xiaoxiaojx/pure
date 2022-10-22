#include "env-inl.h"
#include "env.h"

#include "v8.h"
#include "uv.h"
#include "util-inl.h"

// #include "util.h"
#include "pure_options.h"
#include "pure.h"
#include <iostream>

#include "pure_binding.h"

namespace pure
{

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
    using v8::Value;

    IsolateData::IsolateData(Isolate *isolate,
                             uv_loop_t *event_loop,
                             v8::Platform *platform)
        : isolate_(isolate),
          event_loop_(event_loop),
          platform_(platform)
    {
        options_.reset(
            new PerIsolateOptions(*(per_process::cli_options->per_isolate)));

        CreateProperties();
    }

    void IsolateData::CreateProperties()
    {
        // TODO
    }

    int const Environment::kNodeContextTag = 0x6e6f64;
    void *const Environment::kNodeContextTagPtr = const_cast<void *>(
        static_cast<const void *>(&Environment::kNodeContextTag));

    std::string GetExecPath(const std::vector<std::string> &argv)
    {
        char exec_path_buf[2 * PATH_MAX];
        size_t exec_path_len = sizeof(exec_path_buf);
        std::string exec_path;
        if (uv_exepath(exec_path_buf, &exec_path_len) == 0)
        {
            exec_path = std::string(exec_path_buf, exec_path_len);
        }
        else
        {
            exec_path = argv[0];
        }

        return exec_path;
    }

    void Environment::InitializeMainContext(Local<Context> context,
                                            const EnvSerializeInfo *env_info)
    {
        context_.Reset(context->GetIsolate(), context);
        AssignToContext(context, ContextInfo(""));
        // if (env_info != nullptr)
        // {
        //     DeserializeProperties(env_info);
        // }
        // else
        // {
        CreateProperties();
        // }
    }

    void Environment::CreateProperties()
    {
        HandleScope handle_scope(isolate_);
        Local<Context> ctx = context();

        {
            Context::Scope context_scope(ctx);
            Local<FunctionTemplate> templ = FunctionTemplate::New(isolate());
            // templ->InstanceTemplate()->SetInternalFieldCount(
            //     BaseObject::kInternalFieldCount);
            // templ->Inherit(BaseObject::GetConstructorTemplate(this));

            set_binding_data_ctor_template(templ);
        }
        // TODO
        // Store primordials setup by the per-context script in the environment.
        //         Local<Object> per_context_bindings =
        //             GetPerContextExports(ctx).ToLocalChecked();
        //         Local<Value> primordials =
        //             per_context_bindings->Get(ctx, primordials_string()).ToLocalChecked();
        //         CHECK(primordials->IsObject());
        //         set_primordials(primordials.As<Object>());

        //         Local<String> prototype_string =
        //             FIXED_ONE_BYTE_STRING(isolate(), "prototype");

        // #define V(EnvPropertyName, PrimordialsPropertyName)                              \
//     {                                                                            \
//         Local<Value> ctor =                                                      \
//             primordials.As<Object>()                                             \
//                 ->Get(ctx,                                                       \
//                       FIXED_ONE_BYTE_STRING(isolate(), PrimordialsPropertyName)) \
//                 .ToLocalChecked();                                               \
//         CHECK(ctor->IsObject());                                                 \
//         Local<Value> prototype =                                                 \
//             ctor.As<Object>()->Get(ctx, prototype_string).ToLocalChecked();      \
//         CHECK(prototype->IsObject());                                            \
//         set_##EnvPropertyName(prototype.As<Object>());                           \
//     }

        //         V(primordials_safe_map_prototype_object, "SafeMap");
        //         V(primordials_safe_set_prototype_object, "SafeSet");
        //         V(primordials_safe_weak_map_prototype_object, "SafeWeakMap");
        //         V(primordials_safe_weak_set_prototype_object, "SafeWeakSet");
        // #undef V

        //         Local<Object> process_object =
        //             node::CreateProcessObject(this).FromMaybe(Local<Object>());
        //         set_process_object(process_object);
    }

    Environment::Environment(IsolateData *isolate_data,
                             Isolate *isolate,
                             const std::vector<std::string> &args,
                             const std::vector<std::string> &exec_args,
                             EnvironmentFlags::Flags flags)
        : isolate_(isolate),
          isolate_data_(isolate_data),
          //   immediate_info_(isolate, MAYBE_FIELD_PTR(env_info, immediate_info)),
          //   tick_info_(isolate, MAYBE_FIELD_PTR(env_info, tick_info)),
          timer_base_(uv_now(isolate_data->event_loop())),
          exec_argv_(exec_args),
          argv_(args),
          exec_path_(GetExecPath(args)),
          //   should_abort_on_uncaught_toggle_(
          //       isolate_,
          //       1,
          //       MAYBE_FIELD_PTR(env_info, should_abort_on_uncaught_toggle)),
          //   stream_base_state_(isolate_,
          //                      StreamBase::kNumStreamBaseStateFields,
          //                      MAYBE_FIELD_PTR(env_info, stream_base_state)),
          environment_start_time_(PERFORMANCE_NOW()),
          flags_(flags)
    {
        // TODO
    }

    Environment::Environment(IsolateData *isolate_data,
                             Local<Context> context,
                             const std::vector<std::string> &args,
                             const std::vector<std::string> &exec_args,
                             EnvironmentFlags::Flags flags)
        : Environment(isolate_data,
                      context->GetIsolate(),
                      args,
                      exec_args,
                      flags)
    {
        InitializeMainContext(context, nullptr);
    }

    Environment::~Environment()
    {
        // TODO
    }

    void Environment::CleanupHandles()
    {
        {
            // Mutex::ScopedLock lock(native_immediates_threadsafe_mutex_);
            // task_queues_async_initialized_ = false;
        }

        // Assert that no Javascript code is invoked.
        Isolate::DisallowJavascriptExecutionScope disallow_js(isolate(),
                                                              Isolate::DisallowJavascriptExecutionScope::THROW_ON_FAILURE);

        // RunAndClearNativeImmediates(true /* skip unrefed SetImmediate()s */);

        // for (ReqWrapBase *request : req_wrap_queue_)
        //     request->Cancel();

        // for (HandleWrap *handle : handle_wrap_queue_)
        //     handle->Close();

        // for (HandleCleanup &hc : handle_cleanup_queue_)
        //     hc.cb_(this, hc.handle_, hc.arg_);
        // handle_cleanup_queue_.clear();

        // while (handle_cleanup_waiting_ != 0 ||
        //        request_waiting_ != 0 ||
        //        !handle_wrap_queue_.IsEmpty())
        {
            uv_run(event_loop(), UV_RUN_ONCE);
        }
    }

    void Environment::AtExit(void (*cb)(void *arg), void *arg)
    {
        // TODO
        // at_exit_functions_.push_front(ExitCallback{cb, arg});
    }

    void Environment::RunCleanup()
    {
        started_cleanup_ = true;
        // TRACE_EVENT0(TRACING_CATEGORY_NODE1(environment), "RunCleanup");
        // bindings_.clear();
        CleanupHandles();

        // TODO
        //  while (!cleanup_hooks_.empty() ||
        //         native_immediates_.size() > 0 ||
        //         native_immediates_threadsafe_.size() > 0 ||
        //         native_immediates_interrupts_.size() > 0)
        //  {
        //      // Copy into a vector, since we can't sort an unordered_set in-place.
        //      std::vector<CleanupHookCallback> callbacks(
        //          cleanup_hooks_.begin(), cleanup_hooks_.end());
        //      // We can't erase the copied elements from `cleanup_hooks_` yet, because we
        //      // need to be able to check whether they were un-scheduled by another hook.

        //     std::sort(callbacks.begin(), callbacks.end(),
        //               [](const CleanupHookCallback &a, const CleanupHookCallback &b)
        //               {
        //                   // Sort in descending order so that the most recently inserted callbacks
        //                   // are run first.
        //                   return a.insertion_order_counter_ > b.insertion_order_counter_;
        //               });

        //     for (const CleanupHookCallback &cb : callbacks)
        //     {
        //         if (cleanup_hooks_.count(cb) == 0)
        //         {
        //             // This hook was removed from the `cleanup_hooks_` set during another
        //             // hook that was run earlier. Nothing to do here.
        //             continue;
        //         }

        //         cb.fn_(cb.arg_);
        //         cleanup_hooks_.erase(cb);
        //     }
        //     CleanupHandles();
        // }

        // for (const int fd : unmanaged_fds_)
        // {
        //     uv_fs_t close_req;
        //     uv_fs_close(nullptr, &close_req, fd, nullptr);
        //     uv_fs_req_cleanup(&close_req);
        // }
    }

    void Environment::RunAtExitCallbacks()
    {
        // TODO
        // TRACE_EVENT0(TRACING_CATEGORY_NODE1(environment), "AtExit");
        // for (ExitCallback at_exit : at_exit_functions_)
        // {
        //     at_exit.cb_(at_exit.arg_);
        // }
        // at_exit_functions_.clear();
    }

    void RunAtExit(Environment *env)
    {
        env->RunAtExitCallbacks();
    }

    void FreeEnvironment(Environment *env)
    {
        Isolate *isolate = env->isolate();
        Isolate::DisallowJavascriptExecutionScope disallow_js(isolate,
                                                              Isolate::DisallowJavascriptExecutionScope::THROW_ON_FAILURE);
        {
            HandleScope handle_scope(isolate); // For env->context().
            Context::Scope context_scope(env->context());
            SealHandleScope seal_handle_scope(isolate);

            env->set_stopping(true);
            // TODO
            // env->stop_sub_worker_contexts();
            env->RunCleanup();
            RunAtExit(env);
        }

        // This call needs to be made while the `Environment` is still alive
        // because we assume that it is available for async tracking in the
        // NodePlatform implementation.

        // TODO
        // MultiIsolatePlatform *platform = env->isolate_data()->platform();
        // if (platform != nullptr)
        //     platform->DrainTasks(isolate);

        delete env;
    }

    bool ShouldAbortOnUncaughtException(Isolate *isolate)
    {
        SealHandleScope scope(isolate);
        Environment *env = Environment::GetCurrent(isolate);
        return env != nullptr &&
               (env->is_main_thread() || !env->is_stopping()) &&
               env->abort_on_uncaught_exception();
        //    env->should_abort_on_uncaught_toggle()[0] &&
        //    !env->inside_should_not_abort_on_uncaught_scope();
    }

    // This runs at runtime, regardless of whether the context
    // is created from a snapshot.
    Maybe<bool> InitializeContextRuntime(Local<Context> context)
    {
        // TODO
        return Just(true);
    }

    Maybe<bool> InitializeContextForSnapshot(Local<Context> context)
    {
        // Isolate *isolate = context->GetIsolate();
        // HandleScope handle_scope(isolate);

        // context->SetEmbedderData(ContextEmbedderIndex::kAllowWasmCodeGeneration,
        //                          True(isolate));

        // return InitializePrimordials(context);
    }

    Maybe<bool> InitializeContext(Local<Context> context)
    {
        // std::cout << "InitializeContext ..." << std::endl;

        // if (InitializeContextForSnapshot(context).IsNothing())
        // {
        //     return Nothing<bool>();
        // }

        return InitializeContextRuntime(context);
    }

    // InitializeContext, because embedders don't necessarily
    // call NewContext and so they will experience breakages.
    Local<Context> NewContext(Isolate *isolate)
    {
        auto context = Context::New(isolate, nullptr);
        if (context.IsEmpty())
        {
            std::cout << "NewContext context.IsEmpty()" << std::endl;
            return context;
        }

        if (InitializeContext(context).IsNothing())
        {
            return Local<Context>();
        }

        return context;
    }

    Maybe<bool> Environment::BootstrapPure()
    {
        HandleScope scope(isolate_);
        // 好吧, 我只想保持和 JavaScript 中相同的用法
        // 虽然在 C 中完全没必要这样绕个弯子来引模块
        // GetInternalBinding(this, "addon_console") = require("addon_console")
        Local<Object> __pure_stdout = binding::GetInternalBinding(this, "addon_console").ToLocalChecked();
        Local<Object> global = context()->Global();
        global->Set(context(), FIXED_ONE_BYTE_STRING(isolate_, "__pure_stdout"), __pure_stdout);
        global->Set(context(), FIXED_ONE_BYTE_STRING(isolate_, "global"), global).Check();
        return Just(true);
    }

    MaybeLocal<Value> Environment::RunBootstrapping()
    {

        HandleScope scope(isolate_);
        std::vector<Local<String>> loaders_params = {};
        std::vector<Local<Value>> loaders_args = {};

        // 注入 addon
        BootstrapPure();

        // 运行 lib
        ExecuteBootstrapper(this, "pure:console", &loaders_params, &loaders_args);

        // 运行 user
        ExecuteBootstrapper(this, argv_.front().c_str(), &loaders_params, &loaders_args);
    }

    void Environment::ExitEnv()
    {
        set_can_call_into_js(false);
        set_stopping(true);
        isolate_->TerminateExecution();
        SetImmediateThreadsafe([](Environment *env)
                               { uv_stop(env->event_loop()); },  CallbackFlags::kRefed);
    }
}
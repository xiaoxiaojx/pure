#include "env-inl.h"
#include "env.h"

#include "v8.h"
#include "uv.h"
#include "util-inl.h"

// #include "util.h"
#include "pure_options.h"
#include "pure.h"
#include "console.h"
#include <iostream>

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
        //     CreateProperties();
        // }
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

    // This runs at runtime, regardless of whether the context
    // is created from a snapshot.
    Maybe<bool> InitializeContextRuntime(Local<Context> context)
    {
        // TODO
        // Isolate *isolate = context->GetIsolate();
        // HandleScope handle_scope(isolate);

        // Delete `Intl.v8BreakIterator`
        // https://github.com/nodejs/node/issues/14909
        // {
        //     Local<String> intl_string =
        //         FIXED_ONE_BYTE_STRING(isolate, "Intl");
        //     Local<String> break_iter_string =
        //         FIXED_ONE_BYTE_STRING(isolate, "v8BreakIterator");

        //     Local<Value> intl_v;
        //     if (!context->Global()
        //              ->Get(context, intl_string)
        //              .ToLocal(&intl_v))
        //     {
        //         return Nothing<bool>();
        //     }

        //     if (intl_v->IsObject() &&
        //         intl_v.As<Object>()
        //             ->Delete(context, break_iter_string)
        //             .IsNothing())
        //     {
        //         return Nothing<bool>();
        //     }
        // }

        // // Delete `Atomics.wake`
        // // https://github.com/nodejs/node/issues/21219
        // {
        //     Local<String> atomics_string =
        //         FIXED_ONE_BYTE_STRING(isolate, "Atomics");
        //     Local<String> wake_string =
        //         FIXED_ONE_BYTE_STRING(isolate, "wake");

        //     Local<Value> atomics_v;
        //     if (!context->Global()
        //              ->Get(context, atomics_string)
        //              .ToLocal(&atomics_v))
        //     {
        //         return Nothing<bool>();
        //     }

        //     if (atomics_v->IsObject() &&
        //         atomics_v.As<Object>()
        //             ->Delete(context, wake_string)
        //             .IsNothing())
        //     {
        //         return Nothing<bool>();
        //     }
        // }

        // // Remove __proto__
        // // https://github.com/nodejs/node/issues/31951
        // Local<Object> prototype;
        // {
        //     Local<String> object_string =
        //         FIXED_ONE_BYTE_STRING(isolate, "Object");
        //     Local<String> prototype_string =
        //         FIXED_ONE_BYTE_STRING(isolate, "prototype");

        //     Local<Value> object_v;
        //     if (!context->Global()
        //              ->Get(context, object_string)
        //              .ToLocal(&object_v))
        //     {
        //         return Nothing<bool>();
        //     }

        //     Local<Value> prototype_v;
        //     if (!object_v.As<Object>()
        //              ->Get(context, prototype_string)
        //              .ToLocal(&prototype_v))
        //     {
        //         return Nothing<bool>();
        //     }

        //     prototype = prototype_v.As<Object>();
        // }

        // Local<String> proto_string =
        //     FIXED_ONE_BYTE_STRING(isolate, "__proto__");

        // if (per_process::cli_options->disable_proto == "delete")
        // {
        //     if (prototype
        //             ->Delete(context, proto_string)
        //             .IsNothing())
        //     {
        //         return Nothing<bool>();
        //     }
        // }
        // else if (per_process::cli_options->disable_proto == "throw")
        // {
        //     Local<Value> thrower;
        //     if (!Function::New(context, ProtoThrower)
        //              .ToLocal(&thrower))
        //     {
        //         return Nothing<bool>();
        //     }

        //     PropertyDescriptor descriptor(thrower, thrower);
        //     descriptor.set_enumerable(false);
        //     descriptor.set_configurable(true);
        //     if (prototype
        //             ->DefineProperty(context, proto_string, descriptor)
        //             .IsNothing())
        //     {
        //         return Nothing<bool>();
        //     }
        // }
        // else if (per_process::cli_options->disable_proto != "")
        // {
        //     // Validated in ProcessGlobalArgs
        //     FatalError("InitializeContextRuntime()",
        //                "invalid --disable-proto mode");
        // }

        return Just(true);
    }

    // Maybe<bool> InitializePrimordials(Local<Context> context)
    // {
    // TODO
    // Run per-context JS files.
    // Isolate *isolate = context->GetIsolate();
    // Context::Scope context_scope(context);
    // Local<Object> exports;

    // Local<String> primordials_string =
    //     FIXED_ONE_BYTE_STRING(isolate, "primordials");
    // Local<String> global_string = FIXED_ONE_BYTE_STRING(isolate, "global");
    // Local<String> exports_string = FIXED_ONE_BYTE_STRING(isolate, "exports");

    // // Create primordials first and make it available to per-context scripts.
    // Local<Object> primordials = Object::New(isolate);
    // if (primordials->SetPrototype(context, Null(isolate)).IsNothing() ||
    //     !GetPerContextExports(context).ToLocal(&exports) ||
    //     exports->Set(context, primordials_string, primordials).IsNothing())
    // {
    //     return Nothing<bool>();
    // }
    // return Nothing<bool>();
    // }

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

    MaybeLocal<Value> Environment::BootstrapInternalLoaders()
    {
        // TODO
        // EscapableHandleScope scope(isolate_);

        // // Create binding loaders
        // std::vector<Local<String>> loaders_params = {
        //     process_string(),
        //     FIXED_ONE_BYTE_STRING(isolate_, "getLinkedBinding"),
        //     FIXED_ONE_BYTE_STRING(isolate_, "getInternalBinding"),
        //     primordials_string()};
        // std::vector<Local<Value>> loaders_args = {
        //     process_object(),
        //     NewFunctionTemplate(binding::GetLinkedBinding)
        //         ->GetFunction(context())
        //         .ToLocalChecked(),
        //     NewFunctionTemplate(binding::GetInternalBinding)
        //         ->GetFunction(context())
        //         .ToLocalChecked(),
        //     primordials()};

        // // Bootstrap internal loaders
        // Local<Value> loader_exports;
        // if (!ExecuteBootstrapper(
        //          this, "internal/bootstrap/loaders", &loaders_params, &loaders_args)
        //          .ToLocal(&loader_exports))
        // {
        //     return MaybeLocal<Value>();
        // }
        // CHECK(loader_exports->IsObject());
        // Local<Object> loader_exports_obj = loader_exports.As<Object>();
        // Local<Value> internal_binding_loader =
        //     loader_exports_obj->Get(context(), internal_binding_string())
        //         .ToLocalChecked();
        // CHECK(internal_binding_loader->IsFunction());
        // set_internal_binding_loader(internal_binding_loader.As<Function>());
        // Local<Value> require =
        //     loader_exports_obj->Get(context(), require_string()).ToLocalChecked();
        // CHECK(require->IsFunction());
        // set_native_module_require(require.As<Function>());

        // return scope.Escape(loader_exports);

        return MaybeLocal<Value>();
    }

    Maybe<bool> Environment::BootstrapPure()
    {
        HandleScope scope(isolate_);
        Local<Object> consoleObj = console::Create(this).ToLocalChecked();
        Local<Object> global = context()->Global();
        global->Set(context(), FIXED_ONE_BYTE_STRING(isolate_, "console"), consoleObj);
        global->Set(context(), FIXED_ONE_BYTE_STRING(isolate_, "global"), global).Check();
        return Just(true);
    }

    // MaybeLocal<Value> Environment::BootstrapNode()
    // {
    //     // EscapableHandleScope scope(isolate_);
    //     // // process, require, internalBinding, primordials
    //     // std::vector<Local<String>> node_params = {
    //     //     process_string(),
    //     //     require_string(),
    //     //     internal_binding_string(),
    //     //     primordials_string()};
    //     // std::vector<Local<Value>> node_args = {
    //     //     process_object(),
    //     //     native_module_require(),
    //     //     internal_binding_loader(),
    //     //     primordials()};

    //     // MaybeLocal<Value> result = ExecuteBootstrapper(
    //     //     this, "internal/bootstrap/node", &node_params, &node_args);

    //     // if (result.IsEmpty())
    //     // {
    //     //     return MaybeLocal<Value>();
    //     // }

    //     // if (!no_browser_globals())
    //     // {
    //     //     result = ExecuteBootstrapper(
    //     //         this, "internal/bootstrap/browser", &node_params, &node_args);

    //     //     if (result.IsEmpty())
    //     //     {
    //     //         return MaybeLocal<Value>();
    //     //     }
    //     // }

    //     // // TODO(joyeecheung): skip these in the snapshot building for workers.
    //     // auto thread_switch_id =
    //     //     is_main_thread() ? "internal/bootstrap/switches/is_main_thread"
    //     //                      : "internal/bootstrap/switches/is_not_main_thread";
    //     // result =
    //     //     ExecuteBootstrapper(this, thread_switch_id, &node_params, &node_args);

    //     // if (result.IsEmpty())
    //     // {
    //     //     return MaybeLocal<Value>();
    //     // }

    //     // auto process_state_switch_id =
    //     //     owns_process_state()
    //     //         ? "internal/bootstrap/switches/does_own_process_state"
    //     //         : "internal/bootstrap/switches/does_not_own_process_state";
    //     // result = ExecuteBootstrapper(
    //     //     this, process_state_switch_id, &node_params, &node_args);

    //     // if (result.IsEmpty())
    //     // {
    //     //     return MaybeLocal<Value>();
    //     // }

    //     // Local<String> env_string = FIXED_ONE_BYTE_STRING(isolate_, "env");
    //     // Local<Object> env_var_proxy;
    //     // if (!CreateEnvVarProxy(context(), isolate_).ToLocal(&env_var_proxy) ||
    //     //     process_object()->Set(context(), env_string, env_var_proxy).IsNothing())
    //     // {
    //     //     return MaybeLocal<Value>();
    //     // }

    //     // return scope.EscapeMaybe(result);
    // }

    MaybeLocal<Value> Environment::RunBootstrapping()
    {

        HandleScope scope(isolate_);
        std::vector<Local<String>> loaders_params = {};
        std::vector<Local<Value>> loaders_args = {};

        BootstrapPure();

        ExecuteBootstrapper(this, argv_.front().c_str(), &loaders_params, &loaders_args);
        //     EscapableHandleScope scope(isolate_);

        //     CHECK(!has_run_bootstrapping_code());

        //     if (BootstrapInternalLoaders().IsEmpty())
        //     {
        //         return MaybeLocal<Value>();
        //     }

        //     Local<Value> result;
        //     if (!BootstrapNode().ToLocal(&result))
        //     {
        //         return MaybeLocal<Value>();
        //     }

        //     // Make sure that no request or handle is created during bootstrap -
        //     // if necessary those should be done in pre-execution.
        //     // Usually, doing so would trigger the checks present in the ReqWrap and
        //     // HandleWrap classes, so this is only a consistency check.
        //     // CHECK(req_wrap_queue()->IsEmpty());
        //     // CHECK(handle_wrap_queue()->IsEmpty());

        //     DoneBootstrapping();

        //     return scope.Escape(result);
        // }

        // MaybeLocal<Value> LoadEnvironment(
        //     Environment *env,
        //     StartExecutionCallback cb)
        // {
        //     env->InitializeLibuv();
        //     env->InitializeDiagnostics();

        //     return StartExecution(env, cb);
        // }

        // MaybeLocal<Value> LoadEnvironment(
        //     Environment *env,
        //     const char *main_script_source_utf8)
        // {
        //     CHECK_NOT_NULL(main_script_source_utf8);
        //     Isolate *isolate = env->isolate();
        //     return LoadEnvironment(
        //         env,
        //         [&](const StartExecutionCallbackInfo &info) -> MaybeLocal<Value>
        //         {
        //             // This is a slightly hacky way to convert UTF-8 to UTF-16.
        //             Local<String> str =
        //                 String::NewFromUtf8(isolate,
        //                                     main_script_source_utf8)
        //                     .ToLocalChecked();
        //             auto main_utf16 = std::make_unique<String::Value>(isolate, str);

        //             // TODO(addaleax): Avoid having a global table for all scripts.
        //             std::string name = "embedder_main_" + std::to_string(env->thread_id());
        //             native_module::NativeModuleEnv::Add(
        //                 name.c_str(),
        //                 UnionBytes(**main_utf16, main_utf16->length()));
        //             env->set_main_utf16(std::move(main_utf16));
        //             std::vector<Local<String>> params = {
        //                 env->process_string(),
        //                 env->require_string()};
        //             std::vector<Local<Value>> args = {
        //                 env->process_object(),
        //                 env->native_module_require()};
        //             return ExecuteBootstrapper(env, name.c_str(), &params, &args);
        //         });
    }
}
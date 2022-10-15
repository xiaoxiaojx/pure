#include "env.h"
#include "env-inl.h"

#include "v8.h"
#include "uv.h"
#include "pure_options.h"

namespace pure
{

    using v8::Array;
    using v8::Boolean;
    using v8::Context;
    using v8::EscapableHandleScope;
    using v8::Function;
    using v8::FunctionTemplate;
    using v8::HandleScope;
    using v8::HeapSpaceStatistics;
    using v8::Integer;
    using v8::Isolate;
    using v8::Local;
    using v8::MaybeLocal;
    using v8::NewStringType;
    using v8::Number;
    using v8::Object;
    using v8::Private;
    using v8::Script;
    using v8::SnapshotCreator;
    using v8::StackTrace;
    using v8::String;
    using v8::Symbol;
    using v8::TracingController;
    using v8::TryCatch;
    using v8::Undefined;
    using v8::Value;

    IsolateData::IsolateData(Isolate *isolate,
                             uv_loop_t *event_loop,
                             MultiIsolatePlatform *platform)
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
                             const EnvSerializeInfo *env_info,
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
                             const EnvSerializeInfo *env_info,
                             EnvironmentFlags::Flags flags)
        : Environment(isolate_data,
                      context->GetIsolate(),
                      args,
                      exec_args,
                      env_info,
                      flags)
    {
        InitializeMainContext(context, env_info);
    }

    Environment::~Environment()
    {
        // TODO
    }

}
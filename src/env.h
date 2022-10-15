#ifndef SRC_ENV_H_
#define SRC_ENV_H_

#include <vector>
#include "v8.h"
#include "uv.h"
#include "pure.h"
#include "util.h"

namespace pure
{
    class PerIsolateOptions;
    class IsolateData;

    struct EnvSerializeInfo
    {
        // TODO
    };

    class Environment
    {
    public:
        // Copy Constructor
        Environment(const Environment &) = delete;
        // Copy Assignment operator
        Environment &operator=(const Environment &) = delete;
        // Move Constructor
        Environment(Environment &&) = delete;
        // Move Assignment operator
        Environment &operator=(Environment &&) = delete;

        // C++右值引用 https://zhuanlan.zhihu.com/p/335994370
        // C++ 语言中，函数的参数和返回值的传递方式有三种：值传递、指针传递和引用传递 https://www.runoob.com/w3cnote/cpp-difference-between-pointers-and-references.html

        // =default 和 =delete https://zhuanlan.zhihu.com/p/173231137
        // = delete 禁止使用编译器默认生成的函数
        // = defaule 在程序员重载了自己上面提到的C++编译器默认生成的函数之后，C++编译器将不在生成这些函数的默认形式。但是C++允许我们使用=default来要求编译器生成一个默认函数

        static inline Environment *GetCurrent(v8::Isolate *isolate);
        static inline Environment *GetCurrent(v8::Local<v8::Context> context);
        static inline Environment *GetCurrent(
            const v8::FunctionCallbackInfo<v8::Value> &info);

        template <typename T>
        static inline Environment *GetCurrent(
            const v8::PropertyCallbackInfo<T> &info);

        inline v8::Local<v8::Context> context() const;

        // Create an Environment without initializing a main Context. Use
        // InitializeMainContext() to initialize a main context for it.
        Environment(IsolateData *isolate_data,
                    v8::Isolate *isolate,
                    const std::vector<std::string> &args,
                    const std::vector<std::string> &exec_args,
                    const EnvSerializeInfo *env_info,
                    EnvironmentFlags::Flags flags);
        void InitializeMainContext(v8::Local<v8::Context> context,
                                   const EnvSerializeInfo *env_info);
        // Create an Environment and initialize the provided main context for it.
        Environment(IsolateData *isolate_data,
                    v8::Local<v8::Context> context,
                    const std::vector<std::string> &args,
                    const std::vector<std::string> &exec_args,
                    const EnvSerializeInfo *env_info,
                    EnvironmentFlags::Flags flags);
        ~Environment();

        struct ContextInfo
        {
            explicit ContextInfo(const std::string &name) : name(name) {}
            const std::string name;
            std::string origin;
            bool is_default = false;
        };

        inline void AssignToContext(v8::Local<v8::Context> context,
                                    const ContextInfo &info);

    private:
        v8::Global<v8::Context> context_;

        static void *const kNodeContextTagPtr;
        static int const kNodeContextTag;

        v8::Isolate *const isolate_;
        IsolateData *const isolate_data_;
        uv_timer_t timer_handle_;
        uv_check_t immediate_check_handle_;
        uv_idle_t immediate_idle_handle_;
        uv_prepare_t idle_prepare_handle_;
        uv_check_t idle_check_handle_;
        uv_async_t task_queues_async_;
        int64_t task_queues_async_refs_ = 0;

        // ImmediateInfo immediate_info_;
        // TickInfo tick_info_;
        const uint64_t timer_base_;
        // std::shared_ptr<KVStore> env_vars_;
        bool printed_error_ = false;
        bool trace_sync_io_ = false;
        bool emit_env_nonstring_warning_ = true;
        bool emit_err_name_warning_ = true;
        bool emit_filehandle_warning_ = true;
        bool source_maps_enabled_ = false;

        size_t async_callback_scope_depth_ = 0;
        std::vector<double> destroy_async_id_list_;

        std::vector<std::string> exec_argv_;
        std::vector<std::string> argv_;
        std::string exec_path_;

        uint64_t environment_start_time_;

        uint64_t flags_;
        uint64_t thread_id_;

        // AliasedUint32Array should_abort_on_uncaught_toggle_;
        // int should_not_abort_scope_counter_ = 0;
    };

    class PURE_EXTERN_PRIVATE IsolateData
    {
    public:
        IsolateData(v8::Isolate *isolate,
                    uv_loop_t *event_loop,
                    MultiIsolatePlatform *platform = nullptr);

        inline uv_loop_t *event_loop() const;
        inline MultiIsolatePlatform *platform() const;

        IsolateData(const IsolateData &) = delete;
        IsolateData &operator=(const IsolateData &) = delete;
        IsolateData(IsolateData &&) = delete;
        IsolateData &operator=(IsolateData &&) = delete;

        inline v8::Isolate *isolate() const;

    private:
        v8::Isolate *const isolate_;
        uv_loop_t *const event_loop_;
        MultiIsolatePlatform *platform_;
        std::shared_ptr<PerIsolateOptions> options_;

        void CreateProperties();
    };
}

#endif // SRC_ENV_H_
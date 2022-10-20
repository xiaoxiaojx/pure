#ifndef SRC_PURE_MAIN_INSTANCE_H_
#define SRC_PURE_MAIN_INSTANCE_H_

#include "uv.h"
#include "pure.h"
#include "env.h"

namespace pure
{
    void FreeEnvironment(Environment *env);

    class PureMainInstance
    {
    public:
        PureMainInstance(
            uv_loop_t *event_loop,
            const std::vector<std::string> &args,
            const std::vector<std::string> &exec_args);

        ~PureMainInstance();

        DeleteFnPtr<Environment, FreeEnvironment> CreateMainEnvironment(
            int *exit_code);

        int Run();
        void Run(int *exit_code, Environment *env);

        std::vector<std::string> args_;
        std::vector<std::string> exec_args_;
        v8::Isolate *isolate_;
        v8::Platform *platform_;
        std::unique_ptr<IsolateData> isolate_data_;
        std::unique_ptr<v8::Isolate::CreateParams> isolate_params_;

        PureMainInstance(const PureMainInstance &) = delete;
        PureMainInstance &operator=(const PureMainInstance &) = delete;
        PureMainInstance(PureMainInstance &&) = delete;
        PureMainInstance &operator=(PureMainInstance &&) = delete;
    };
}

#endif // SRC_PURE_MAIN_INSTANCE_H_

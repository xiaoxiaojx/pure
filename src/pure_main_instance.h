#ifndef SRC_PURE_MAIN_INSTANCE_H_
#define SRC_PURE_MAIN_INSTANCE_H_

#include "uv.h"
#include "pure.h"
#include "env.h"

namespace pure
{
    class PureMainInstance
    {
    public:
        // To create a main instance that does not own the isolate,
        // The caller needs to do:
        //
        //   Isolate* isolate = Isolate::Allocate();
        //   platform->RegisterIsolate(isolate, loop);
        //   isolate->Initialize(...);
        //   isolate->Enter();
        //   std::unique_ptr<PureMainInstance> main_instance =
        //       PureMainInstance::Create(isolate, loop, args, exec_args);
        //
        // When tearing it down:
        //
        //   main_instance->Cleanup();  // While the isolate is entered
        //   isolate->Exit();
        //   isolate->Dispose();
        //   platform->UnregisterIsolate(isolate);
        //
        // After calling Dispose() the main_instance is no longer accessible.

        // static std::unique_ptr<PureMainInstance> Create(
        //     v8::Isolate *isolate,
        //     uv_loop_t *event_loop,
        //     MultiIsolatePlatform *platform,
        //     const std::vector<std::string> &args,
        //     const std::vector<std::string> &exec_args);

        // void Dispose();

        // // Create a main instance that owns the isolate
        // PureMainInstance(uv_loop_t *event_loop,
        //                  MultiIsolatePlatform *platform,
        //                  const std::vector<std::string> &args,
        //                  const std::vector<std::string> &exec_args);
        // ~PureMainInstance();

        // // Start running the Node.js instances, return the exit code when finished.
        // int Run();
        // void Run(int *exit_code, Environment *env);

        // IsolateData *isolate_data() { return isolate_data_.get(); }

        // DeleteFnPtr<Environment, FreeEnvironment> CreateMainEnvironment(
        //     int *exit_code);

        // PureMainInstance(const PureMainInstance &) = delete;
        // PureMainInstance &operator=(const PureMainInstance &) = delete;
        // PureMainInstance(PureMainInstance &&) = delete;
        // PureMainInstance &operator=(PureMainInstance &&) = delete;

    public:
        PureMainInstance(
            uv_loop_t *event_loop,
            MultiIsolatePlatform *platform,
            const std::vector<std::string> &args,
            const std::vector<std::string> &exec_args);

        // static std::unique_ptr<ExternalReferenceRegistry> registry_;
        std::vector<std::string> args_;
        std::vector<std::string> exec_args_;
        v8::Isolate *isolate_;
        MultiIsolatePlatform *platform_;
        std::unique_ptr<IsolateData> isolate_data_;
        std::unique_ptr<v8::Isolate::CreateParams> isolate_params_;
        // const SnapshotData *snapshot_data_ = nullptr;

        PureMainInstance(const PureMainInstance &) = delete;
        PureMainInstance &operator=(const PureMainInstance &) = delete;
        PureMainInstance(PureMainInstance &&) = delete;
        PureMainInstance &operator=(PureMainInstance &&) = delete;
    };
}

#endif // SRC_PURE_MAIN_INSTANCE_H_

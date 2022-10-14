#include "pure.h"
#include "uv.h"
#include <iostream>
namespace pure
{
    InitializationResult InitializeOncePerProcess(
        int argc,
        char **argv,
        InitializationSettingsFlags flags)
    {
        InitializationResult result;

        return result;
    }

    InitializationResult InitializeOncePerProcess(int argc, char **argv)
    {
        return InitializeOncePerProcess(argc, argv, kDefaultInitialization);
    }

    int Start(int argc, char **argv)
    {
        InitializationResult result = InitializeOncePerProcess(argc, argv);
        if (result.early_return)
        {
            return result.exit_code;
        }

        //   if (per_process::cli_options->build_snapshot) {
        //     fprintf(stderr,
        //             "--build-snapshot is not yet supported in the node binary\n");
        //     return 1;
        //   }

        //   {
        //     bool use_node_snapshot = per_process::cli_options->node_snapshot;
        //     const SnapshotData* snapshot_data =
        //         use_node_snapshot ? SnapshotBuilder::GetEmbeddedSnapshotData()
        //                           : nullptr;
        //     uv_loop_configure(uv_default_loop(), UV_METRICS_IDLE_TIME);

        //     if (snapshot_data != nullptr) {
        //       native_module::NativeModuleEnv::RefreshCodeCache(
        //           snapshot_data->code_cache);
        //     }
        //     NodeMainInstance main_instance(snapshot_data,
        //                                    uv_default_loop(),
        //                                    per_process::v8_platform.Platform(),
        //                                    result.args,
        //                                    result.exec_args);
        //     result.exit_code = main_instance.Run();
        //   }

        //   TearDownOncePerProcess();

        std::cout << "Pure > Start End!\n";

        return result.exit_code;
    }

    // Safe to call more than once and from signal handlers.
    void ResetStdio()
    {
        uv_tty_reset_mode();
    }

}

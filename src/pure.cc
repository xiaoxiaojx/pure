//
//
//  ________  ___  ___  ________  _______
// |\   __  \|\  \|\  \|\   __  \|\  ___ \     
// \ \  \|\  \ \  \\\  \ \  \|\  \ \   __/|
//  \ \   ____\ \  \\\  \ \   _  _\ \  \_|/__
//   \ \  \___|\ \  \\\  \ \  \\  \\ \  \_|\ \ 
//    \ \__\    \ \_______\ \__\\ _\\ \_______\
//     \|__|     \|_______|\|__|\|__|\|_______|
//
//
//

#include <iostream>

#include "pure.h"
#include "uv.h"
#include "v8.h"
#include "util.h"

#include "libplatform/libplatform.h"

#include "pure_main_instance.h"
namespace pure
{
    using v8::V8;

    namespace per_process
    {
        // util.h
        // Tells whether the per-process V8::Initialize() is called and
        // if it is safe to call v8::Isolate::TryGetCurrent().
        bool v8_initialized = false;

        std::unique_ptr<v8::Platform> v8_platform;
    }
    // Safe to call more than once and from signal handlers.
    void ResetStdio()
    {
        // uv_tty_reset_mode http://docs.libuv.org/en/v1.x/tty.html?highlight=uv_tty_reset_mode#c.uv_tty_reset_mode
        // tcgetattr函数与tcsetattr函数控制终端 https://www.cnblogs.com/zhouhbing/p/4129280.html
        // 如果在某处通过 uv_tty_set_mode 修改了终端参数, 此处用于复原
        uv_tty_reset_mode();
    }

    InitializationResult InitializeOncePerProcess(
        int argc,
        char **argv,
        InitializationSettingsFlags flags)
    {
        InitializationResult result;

        // atexit 类似于 Node.js process.on("exit", fn)
        atexit(ResetStdio);

        per_process::v8_platform = v8::platform::NewDefaultPlatform();

        V8::InitializePlatform(per_process::v8_platform.get());

        V8::Initialize();

        // TODO

        return result;
    }

    InitializationResult InitializeOncePerProcess(int argc, char **argv)
    {
        return InitializeOncePerProcess(argc, argv, kDefaultInitialization);
    }

    void TearDownOncePerProcess()
    {
        per_process::v8_initialized = false;
        V8::Dispose();

        // uv_run cannot be called from the time before the beforeExit callback
        // runs until the program exits unless the event loop has any referenced
        // handles after beforeExit terminates. This prevents unrefed timers
        // that happen to terminate during shutdown from being run unsafely.
        // Since uv_run cannot be called, uv_async handles held by the platform
        // will never be fully cleaned up.
        std::unique_ptr<v8::Platform> platform = std::move(per_process::v8_platform);
        // platform.Dispose();
    }

    int Start(int argc, char **argv)
    {
        InitializationResult result = InitializeOncePerProcess(argc, argv);
        if (result.early_return)
        {
            return result.exit_code;
        }

        uv_loop_configure(uv_default_loop(), UV_METRICS_IDLE_TIME);

        PureMainInstance main_instance(
            uv_default_loop(),
            result.args,
            result.exec_args);
        // result.exit_code = main_instance.Run();

        //   TearDownOncePerProcess();

        std::cout << "Pure > Start End!\n";

        return result.exit_code;
    }
}

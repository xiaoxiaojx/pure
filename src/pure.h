#ifndef SRC_PURE_H_
#define SRC_PURE_H_

#include <string>
#include <vector>
#include "v8.h"

#ifdef _WIN32
#ifndef BUILDING_NODE_EXTENSION
#define PURE_EXTERN __declspec(dllexport)
#else
#define PURE_EXTERN __declspec(dllimport)
#endif
#else
#define PURE_EXTERN __attribute__((visibility("default")))
#endif

#ifdef NODE_SHARED_MODE
#define PURE_EXTERN_PRIVATE PURE_EXTERN
#else
#define PURE_EXTERN_PRIVATE
#endif

namespace pure
{

#define PERFORMANCE_NOW() uv_hrtime()

#ifndef NODE_CONTEXT_EMBEDDER_DATA_INDEX
#define NODE_CONTEXT_EMBEDDER_DATA_INDEX 32
#endif

#ifndef NODE_CONTEXT_SANDBOX_OBJECT_INDEX
#define NODE_CONTEXT_SANDBOX_OBJECT_INDEX 33
#endif

#ifndef NODE_CONTEXT_ALLOW_WASM_CODE_GENERATION_INDEX
#define NODE_CONTEXT_ALLOW_WASM_CODE_GENERATION_INDEX 34
#endif

#ifndef NODE_CONTEXT_TAG
#define NODE_CONTEXT_TAG 35
#endif

#ifndef NODE_BINDING_LIST
#define NODE_BINDING_LIST_INDEX 36
#endif

    // PURE_EXTERN https://www.jianshu.com/p/1e6315145fcf
    // GNU C 的一大特色就是attribute 机制。
    // 试想这样的情景，程序调用某函数A，A函数存在于两个动态链接库liba.so,libb.so中，并且程序执行需要链接这两个库，此时程序调用的A函数到底是来自于a还是b呢？
    // 这取决于链接时的顺序，比如先链接liba.so，这时候通过liba.so的导出符号表就可以找到函数A的定义，并加入到符号表中，链接libb.so的时候，符号表中已经存在函数A，就不会再更新符号表，所以调用的始终是liba.so中的A函数。
    // 为了避免这种混乱，所以使用
    // __attribute__((visibility("default")))  //默认，设置为：default之后就可以让外面的类看见了。
    // __attribute__((visibility("hideen")))  //隐藏
    PURE_EXTERN int Start(int argc, char *argv[]);

    struct InitializationResult
    {
        int exit_code = 0;
        std::vector<std::string> args;
        std::vector<std::string> exec_args;
        bool early_return = false;
    };

    enum InitializationSettingsFlags : uint64_t
    {
        kDefaultInitialization = 1 << 0,
        kInitializeV8 = 1 << 1,
        kRunPlatformInit = 1 << 2,
        kInitOpenSSL = 1 << 3
    };

    namespace ProcessFlags
    {
        enum Flags : uint64_t
        {
            kNoFlags = 0,
            // Enable stdio inheritance, which is disabled by default.
            kEnableStdioInheritance = 1 << 0,
            // Disable reading the NODE_OPTIONS environment variable.
            kDisableNodeOptionsEnv = 1 << 1,
            // Do not parse CLI options.
            kDisableCLIOptions = 1 << 2,
            // Do not initialize ICU.
            kNoICU = 1 << 3,
        };
    } // namespace ProcessFlags

    class PURE_EXTERN IsolatePlatformDelegate
    {
    public:
        // virtual https://www.runoob.com/w3cnote/cpp-virtual-functions.html
        // 定义一个函数为虚函数，不代表函数为不被实现的函数。
        // 定义他为虚函数是为了允许用基类的指针来调用子类的这个函数。
        // 定义一个函数为纯虚函数，才代表函数没有被实现。
        // 定义纯虚函数是为了实现一个接口，起到一个规范的作用，规范继承这个类的程序员必须实现这个函数
        virtual std::shared_ptr<v8::TaskRunner> GetForegroundTaskRunner() = 0;
        virtual bool IdleTasksEnabled() = 0;
    };

    // class PURE_EXTERN MultiIsolatePlatform : public v8::Platform
    // {
    // public:
    //     ~MultiIsolatePlatform() override = default;
    //     // Returns true if work was dispatched or executed. New tasks that are
    //     // posted during flushing of the queue are postponed until the next
    //     // flushing.
    //     virtual bool FlushForegroundTasks(v8::Isolate *isolate) = 0;
    //     virtual void DrainTasks(v8::Isolate *isolate) = 0;

    //     // This needs to be called between the calls to `Isolate::Allocate()` and
    //     // `Isolate::Initialize()`, so that initialization can already start
    //     // using the platform.
    //     // When using `NewIsolate()`, this is taken care of by that function.
    //     // This function may only be called once per `Isolate`.
    //     virtual void RegisterIsolate(v8::Isolate *isolate,
    //                                  struct uv_loop_s *loop) = 0;
    //     // This method can be used when an application handles task scheduling on its
    //     // own through `IsolatePlatformDelegate`. Upon registering an isolate with
    //     // this overload any other method in this class with the exception of
    //     // `UnregisterIsolate` *must not* be used on that isolate.
    //     virtual void RegisterIsolate(v8::Isolate *isolate,
    //                                  IsolatePlatformDelegate *delegate) = 0;

    //     // This function may only be called once per `Isolate`, and discard any
    //     // pending delayed tasks scheduled for that isolate.
    //     // This needs to be called right before calling `Isolate::Dispose()`.
    //     virtual void UnregisterIsolate(v8::Isolate *isolate) = 0;

    //     // The platform should call the passed function once all state associated
    //     // with the given isolate has been cleaned up. This can, but does not have to,
    //     // happen asynchronously.
    //     virtual void AddIsolateFinishedCallback(v8::Isolate *isolate,
    //                                             void (*callback)(void *),
    //                                             void *data) = 0;

    //     static std::unique_ptr<MultiIsolatePlatform> Create(
    //         int thread_pool_size,
    //         v8::TracingController *tracing_controller = nullptr,
    //         v8::PageAllocator *page_allocator = nullptr);
    // };

    enum IsolateSettingsFlags
    {
        MESSAGE_LISTENER_WITH_ERROR_LEVEL = 1 << 0,
        DETAILED_SOURCE_POSITIONS_FOR_PROFILING = 1 << 1,
        SHOULD_NOT_SET_PROMISE_REJECTION_CALLBACK = 1 << 2,
        SHOULD_NOT_SET_PREPARE_STACK_TRACE_CALLBACK = 1 << 3
    };

    struct IsolateSettings
    {
        uint64_t flags = MESSAGE_LISTENER_WITH_ERROR_LEVEL |
                         DETAILED_SOURCE_POSITIONS_FOR_PROFILING;
        v8::MicrotasksPolicy policy = v8::MicrotasksPolicy::kExplicit;

        // Error handling callbacks
        v8::Isolate::AbortOnUncaughtExceptionCallback
            should_abort_on_uncaught_exception_callback = nullptr;
        v8::FatalErrorCallback fatal_error_callback = nullptr;
        v8::PrepareStackTraceCallback prepare_stack_trace_callback = nullptr;

        // Miscellaneous callbacks
        v8::PromiseRejectCallback promise_reject_callback = nullptr;
        v8::AllowWasmCodeGenerationCallback
            allow_wasm_code_generation_callback = nullptr;
    };

    PURE_EXTERN void PromiseRejectCallback(v8::PromiseRejectMessage message);

    void ResetStdio(); // Safe to call more than once and from signal handlers.

    enum ContextEmbedderIndex
    {
        kEnvironment = NODE_CONTEXT_EMBEDDER_DATA_INDEX,
        kSandboxObject = NODE_CONTEXT_SANDBOX_OBJECT_INDEX,
        kAllowWasmCodeGeneration = NODE_CONTEXT_ALLOW_WASM_CODE_GENERATION_INDEX,
        kContextTag = NODE_CONTEXT_TAG,
        kBindingListIndex = NODE_BINDING_LIST_INDEX
    };

    namespace EnvironmentFlags
    {
        enum Flags : uint64_t
        {
            kNoFlags = 0,
            // Use the default behaviour for Node.js instances.
            kDefaultFlags = 1 << 0,
            // Controls whether this Environment is allowed to affect per-process state
            // (e.g. cwd, process title, uid, etc.).
            // This is set when using kDefaultFlags.
            kOwnsProcessState = 1 << 1,
            // Set if this Environment instance is associated with the global inspector
            // handling code (i.e. listening on SIGUSR1).
            // This is set when using kDefaultFlags.
            kOwnsInspector = 1 << 2,
            // Set if Node.js should not run its own esm loader. This is needed by some
            // embedders, because it's possible for the Node.js esm loader to conflict
            // with another one in an embedder environment, e.g. Blink's in Chromium.
            kNoRegisterESMLoader = 1 << 3,
            // Set this flag to make Node.js track "raw" file descriptors, i.e. managed
            // by fs.open() and fs.close(), and close them during FreeEnvironment().
            kTrackUnmanagedFds = 1 << 4,
            // Set this flag to force hiding console windows when spawning child
            // processes. This is usually used when embedding Node.js in GUI programs on
            // Windows.
            kHideConsoleWindows = 1 << 5,
            // Set this flag to disable loading native addons via `process.dlopen`.
            // This environment flag is especially important for worker threads
            // so that a worker thread can't load a native addon even if `execArgv`
            // is overwritten and `--no-addons` is not specified but was specified
            // for this Environment instance.
            kNoNativeAddons = 1 << 6,
            // Set this flag to disable searching modules from global paths like
            // $HOME/.node_modules and $NODE_PATH. This is used by standalone apps that
            // do not expect to have their behaviors changed because of globally
            // installed modules.
            kNoGlobalSearchPaths = 1 << 7,
            // Do not export browser globals like setTimeout, console, etc.
            kNoBrowserGlobals = 1 << 8,
            // Controls whether or not the Environment should call V8Inspector::create().
            // This control is needed by embedders who may not want to initialize the V8
            // inspector in situations where one has already been created,
            // e.g. Blink's in Chromium.
            kNoCreateInspector = 1 << 9
        };
    } // namespace EnvironmentFlags

}

#endif // SRC_PURE_H_
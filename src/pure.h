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

    class PURE_EXTERN MultiIsolatePlatform : public v8::Platform
    {
    public:
        ~MultiIsolatePlatform() override = default;
        // Returns true if work was dispatched or executed. New tasks that are
        // posted during flushing of the queue are postponed until the next
        // flushing.
        virtual bool FlushForegroundTasks(v8::Isolate *isolate) = 0;
        virtual void DrainTasks(v8::Isolate *isolate) = 0;

        // This needs to be called between the calls to `Isolate::Allocate()` and
        // `Isolate::Initialize()`, so that initialization can already start
        // using the platform.
        // When using `NewIsolate()`, this is taken care of by that function.
        // This function may only be called once per `Isolate`.
        virtual void RegisterIsolate(v8::Isolate *isolate,
                                     struct uv_loop_s *loop) = 0;
        // This method can be used when an application handles task scheduling on its
        // own through `IsolatePlatformDelegate`. Upon registering an isolate with
        // this overload any other method in this class with the exception of
        // `UnregisterIsolate` *must not* be used on that isolate.
        virtual void RegisterIsolate(v8::Isolate *isolate,
                                     IsolatePlatformDelegate *delegate) = 0;

        // This function may only be called once per `Isolate`, and discard any
        // pending delayed tasks scheduled for that isolate.
        // This needs to be called right before calling `Isolate::Dispose()`.
        virtual void UnregisterIsolate(v8::Isolate *isolate) = 0;

        // The platform should call the passed function once all state associated
        // with the given isolate has been cleaned up. This can, but does not have to,
        // happen asynchronously.
        virtual void AddIsolateFinishedCallback(v8::Isolate *isolate,
                                                void (*callback)(void *),
                                                void *data) = 0;

        static std::unique_ptr<MultiIsolatePlatform> Create(
            int thread_pool_size,
            v8::TracingController *tracing_controller = nullptr,
            v8::PageAllocator *page_allocator = nullptr);
    };

    class PureArrayBufferAllocator;
    class IsolateData;

    // An ArrayBuffer::Allocator class with some Node.js-specific tweaks. If you do
    // not have to use another allocator, using this class is recommended:
    // - It supports Buffer.allocUnsafe() and Buffer.allocUnsafeSlow() with
    //   uninitialized memory.
    // - It supports transferring, rather than copying, ArrayBuffers when using
    //   MessagePorts.
    class PURE_EXTERN ArrayBufferAllocator : public v8::ArrayBuffer::Allocator
    {
    public:
        // If `always_debug` is true, create an ArrayBuffer::Allocator instance
        // that performs additional integrity checks (e.g. make sure that only memory
        // that was allocated by the it is also freed by it).
        // This can also be set using the --debug-arraybuffer-allocations flag.
        static std::unique_ptr<ArrayBufferAllocator> Create(
            bool always_debug = false);

    private:
        virtual PureArrayBufferAllocator *GetImpl() = 0;

        friend class IsolateData;
    };

}

#endif // SRC_PURE_H_
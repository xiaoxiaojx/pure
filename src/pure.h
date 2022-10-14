#include <string>
#include <vector>

// PURE_EXTERN https://www.jianshu.com/p/1e6315145fcf
// GNU C 的一大特色就是attribute 机制。
// 试想这样的情景，程序调用某函数A，A函数存在于两个动态链接库liba.so,libb.so中，并且程序执行需要链接这两个库，此时程序调用的A函数到底是来自于a还是b呢？
// 这取决于链接时的顺序，比如先链接liba.so，这时候通过liba.so的导出符号表就可以找到函数A的定义，并加入到符号表中，链接libb.so的时候，符号表中已经存在函数A，就不会再更新符号表，所以调用的始终是liba.so中的A函数。
// 为了避免这种混乱，所以使用
// __attribute__((visibility("default")))  //默认，设置为：default之后就可以让外面的类看见了。
// __attribute__((visibility("hideen")))  //隐藏
#ifdef _WIN32
#ifndef BUILDING_NODE_EXTENSION
#define PURE_EXTERN __declspec(dllexport)
#else
#define PURE_EXTERN __declspec(dllimport)
#endif
#else
#define PURE_EXTERN __attribute__((visibility("default")))
#endif

namespace pure
{
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
}
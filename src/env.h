#ifndef SRC_ENV_H_
#define SRC_ENV_H_

#include <vector>
#include "v8.h"
#include "uv.h"
#include "pure.h"

namespace pure
{
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

        // 一文读懂C++右值引用 https://zhuanlan.zhihu.com/p/335994370
        // C++ 语言中，函数的参数和返回值的传递方式有三种：值传递、指针传递和引用传递 https://www.runoob.com/w3cnote/cpp-difference-between-pointers-and-references.html

        // =default 和 =delete https://zhuanlan.zhihu.com/p/173231137
        // = delete 禁止使用编译器默认生成的函数
        // = defaule 在程序员重载了自己上面提到的C++编译器默认生成的函数之后，C++编译器将不在生成这些函数的默认形式。但是C++允许我们使用=default来要求编译器生成一个默认函数
    };

    class PURE_EXTERN_PRIVATE IsolateData
    {
    public:
        IsolateData(v8::Isolate *isolate,
                    uv_loop_t *event_loop,
                    MultiIsolatePlatform *platform = nullptr,
                    ArrayBufferAllocator *node_allocator = nullptr,
                    const std::vector<size_t> *indexes = nullptr);

        inline uv_loop_t *event_loop() const;
        inline MultiIsolatePlatform *platform() const;
        inline PureArrayBufferAllocator *node_allocator() const;

        IsolateData(const IsolateData &) = delete;
        IsolateData &operator=(const IsolateData &) = delete;
        IsolateData(IsolateData &&) = delete;
        IsolateData &operator=(IsolateData &&) = delete;
    };
}

#endif // SRC_ENV_H_
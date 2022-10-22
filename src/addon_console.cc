/**
 * @file addon_console.cc
 * @author xiaoxiaojx (784487301@qq.com)
 * @brief 为 JavaScript 提供向标准输出写入数据能力的 binding
 * @version 0.1
 * @date 2022-10-22
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "v8.h"
#include "env-inl.h"
#include "util-inl.h"
#include "pure_binding.h"

#include <stdio.h>

namespace pure
{
    namespace addon_console
    {
        using v8::Context;
        using v8::EscapableHandleScope;
        using v8::FunctionCallbackInfo;
        using v8::Isolate;
        using v8::Local;
        using v8::MaybeLocal;
        using v8::ObjectTemplate;
        using v8::String;
        using v8::Value;

        // 未加 static 修饰的函数和全局变量具有全局可见性，其他的源文件也能够访问
        // static 修饰函数和变量这一特性可以在不同的文件中定义同名函数和同名变量，而不必担心命名冲突
        static void Write(const FunctionCallbackInfo<Value> &args)
        {
            v8::Local<v8::Context> context = args.GetIsolate()->GetCurrentContext();

            // As 语法的实现
            // 所以 As<String>() == String(_env, _value)
            // template <typename T>
            // inline T Value::As() const
            // {
            //     return T(_env, _value);
            // }
            Local<String> arg0 = args[0].As<String>();

            // Local<String> 调用 WriteUtf8 方法写出数据到 Utf8Value 的 buf_ 中
            // 即完成了数据的提取(转换)
            // Local<String> -> char *
            Utf8Value string(args.GetIsolate(), arg0);

            // 好吧, 👀 凑合先用 printf 实现 console 吧
            printf("%s \n", *string);
        }

        // Pure C++ addon 先保持和 Node.js 同样的初始化的接口
        void Initialize(Local<Object> target,
                        Local<Value> unused,
                        Local<Context> context,
                        void *priv)
        {
            Environment *env = Environment::GetCurrent(context);
            env->SetMethod(target, "write", Write);
        }
    }
}

// Pure Addon Register
PURE_MODULE_CONTEXT_AWARE_INTERNAL(addon_console, pure::addon_console::Initialize);

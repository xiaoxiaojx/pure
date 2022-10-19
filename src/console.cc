//                              Pure Addon
//
//  ________  ________  ________   ________  ________  ___       _______
// |\   ____\|\   __  \|\   ___  \|\   ____\|\   __  \|\  \     |\  ___ \     
// \ \  \___|\ \  \|\  \ \  \\ \  \ \  \___|\ \  \|\  \ \  \    \ \   __/|
//  \ \  \    \ \  \\\  \ \  \\ \  \ \_____  \ \  \\\  \ \  \    \ \  \_|/__
//   \ \  \____\ \  \\\  \ \  \\ \  \|____|\  \ \  \\\  \ \  \____\ \  \_|\ \ 
//    \ \_______\ \_______\ \__\\ \__\____\_\  \ \_______\ \_______\ \_______\
//     \|_______|\|_______|\|__| \|__|\_________\|_______|\|_______|\|_______|
//                                   \|_________|

#include "v8.h"
#include "env-inl.h"
#include "util-inl.h"
#include "pure_binding.h"

#include <stdio.h>

namespace pure
{
    namespace console
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
        static void Log(const FunctionCallbackInfo<Value> &args)
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
            env->SetMethod(target, "log", Log);
            env->SetMethod(target, "info", Log);
            env->SetMethod(target, "warn", Log);
            env->SetMethod(target, "error", Log);
        }
    }
}

// Pure Addon Register
PURE_MODULE_CONTEXT_AWARE_INTERNAL(console, pure::console::Initialize);

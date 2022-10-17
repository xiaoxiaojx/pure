#include "v8.h"
#include "env-inl.h"
#include "util-inl.h"

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

        static void Log(const FunctionCallbackInfo<Value> &args)
        {
            v8::Local<v8::Context> context = args.GetIsolate()->GetCurrentContext();

            Local<String> arg0 = args[0].As<String>();

            Utf8Value string(args.GetIsolate(), arg0);

            printf("%s \n", *string);
        }

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

        MaybeLocal<Object> Create(Environment *env)
        {
            Isolate *isolate = env->isolate();
            EscapableHandleScope scope(isolate);
            Local<Context> context = env->context();

            Local<ObjectTemplate> console_template = ObjectTemplate::New(isolate);
            Local<Object> console;
            Local<Value> val;

            if (!console_template->NewInstance(context).ToLocal(&console))
            {
                return MaybeLocal<Object>();
            }

            Initialize(console, val, context, nullptr);
            return scope.Escape(console);
        }
    }
}

#ifndef SRC_CONSOLE_H_
#define SRC_CONSOLE_H_

#include "v8.h"
#include "pure.h"

namespace pure
{
    namespace console
    {
        using v8::Context;
        using v8::Local;
        using v8::MaybeLocal;
        using v8::Object;
        using v8::Value;

        void Initialize(Local<Object> target,
                        Local<Value> unused,
                        Local<Context> context,
                        void *priv);

        MaybeLocal<Object> Create(Environment *env);
    }
}

#endif // SRC_CONSOLE_H_

#ifndef SRC_PURE_NATIVE_MODULE_H_
#define SRC_PURE_NATIVE_MODULE_H_

#include "v8.h"
#include "env.h"

namespace pure
{
    namespace native_module
    {

        v8::MaybeLocal<v8::String> LoadBuiltinModuleSource(v8::Isolate *isolate,
                                                           const char *id);

        v8::MaybeLocal<v8::Function> LookupAndCompile(
            v8::Local<v8::Context> context,
            const char *id,
            std::vector<v8::Local<v8::String>> *parameters,
            Environment *optional_env);
    }
}

#endif // SRC_PURE_NATIVE_MODULE_H_

#ifndef SRC_PURE_NATIVE_MODULE_H_
#define SRC_PURE_NATIVE_MODULE_H_

#include "v8.h"
#include "env.h"
#include "pure.h"
#include <map>
#include "pure_union_bytes.h"
namespace pure
{
    namespace native_module
    {
        using NativeModuleRecordMap = std::map<std::string, UnionBytes>;

        class PURE_EXTERN_PRIVATE NativeModuleLoader
        {
        public:
            NativeModuleLoader(const NativeModuleLoader &) = delete;
            NativeModuleLoader &operator=(const NativeModuleLoader &) = delete;

            NativeModuleLoader();
            static NativeModuleLoader *GetInstance();

            void LoadJavaScriptSource();

            v8::MaybeLocal<v8::String>
            LoadBuiltinModuleSource(v8::Isolate *isolate,
                                    const char *id);

            v8::MaybeLocal<v8::String> LoadUserModuleSource(v8::Isolate *isolate,
                                                            const char *id);

            v8::MaybeLocal<v8::Function> LookupAndCompile(
                v8::Local<v8::Context> context,
                const char *id,
                std::vector<v8::Local<v8::String>> *parameters,
                Environment *optional_env);

        private:
            static NativeModuleLoader instance_;
            NativeModuleRecordMap source_;
        };
    }
}

#endif // SRC_PURE_NATIVE_MODULE_H_

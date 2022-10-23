#ifndef SRC_PURE_BINDING_H_
#define SRC_PURE_BINDING_H_

#include "env.h"
#include "pure.h"

enum {
  NM_F_BUILTIN = 1 << 0,  // Unused.
  NM_F_LINKED = 1 << 1,
  NM_F_INTERNAL = 1 << 2,
  NM_F_DELETEME = 1 << 3,
};

#define PURE_MODULE_CONTEXT_AWARE_CPP(modname, regfunc, priv, flags)           \
  static pure::pure_module _module = {                                         \
      PURE_MODULE_VERSION,                                                     \
      flags,                                                                   \
      nullptr,                                                                 \
      __FILE__,                                                                \
      nullptr,                                                                 \
      (pure::addon_context_register_func)(regfunc),                            \
      PURE_STRINGIFY(modname),                                                 \
      priv,                                                                    \
      nullptr};                                                                \
  void _register_##modname() {                                                 \
    pure_module_register(&_module);                                            \
  }

namespace pure {

#define PURE_MODULE_CONTEXT_AWARE_INTERNAL(modname, regfunc)                   \
  PURE_MODULE_CONTEXT_AWARE_CPP(modname, regfunc, nullptr, NM_F_INTERNAL)

namespace binding {
using v8::FunctionCallbackInfo;
using v8::MaybeLocal;
using v8::Object;
using v8::Value;

void RegisterBuiltinModules();
void GetInternalBinding(const FunctionCallbackInfo<Value>& args);

MaybeLocal<Object> GetInternalBinding(Environment* env, const char*);
}  // namespace binding

}  // namespace pure

#endif  // SRC_PURE_BINDING_H_
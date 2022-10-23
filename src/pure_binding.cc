#include "env-inl.h"

#include "pure_binding.h"
#include "util-inl.h"
#include "util.h"
#include "v8.h"

#define PURE_BUILTIN_STANDARD_MODULES(V) V(addon_console)

#define PURE_BUILTIN_MODULES(V) PURE_BUILTIN_STANDARD_MODULES(V)

#define V(modname) void _register_##modname();
PURE_BUILTIN_MODULES(V)
#undef V

namespace pure {
namespace binding {
using v8::Context;
using v8::EscapableHandleScope;
using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::Local;
using v8::MaybeLocal;
using v8::Object;
using v8::String;
using v8::Value;

// Globals per process
static pure_module* modlist_internal;
static pure_module* modlist_linked;
static thread_local pure_module* thread_local_modpending;

// This is set by node::Init() which is used by embedders
bool node_is_initialized = false;

// TODO
extern "C" void pure_module_register(void* m)
// void pure_module_register(void *m)
{
  struct pure_module* mp = reinterpret_cast<struct pure_module*>(m);

  if (mp->nm_flags & NM_F_INTERNAL) {
    mp->nm_link = modlist_internal;
    modlist_internal = mp;
  } else if (!node_is_initialized) {
    // "Linked" modules are included as part of the node project.
    // Like builtins they are registered *before* node::Init runs.
    mp->nm_flags = NM_F_LINKED;
    mp->nm_link = modlist_linked;
    modlist_linked = mp;
  } else {
    thread_local_modpending = mp;
  }
}

inline struct pure_module* FindModule(struct pure_module* list,
                                      const char* name,
                                      int flag) {
  struct pure_module* mp;

  for (mp = list; mp != nullptr; mp = mp->nm_link) {
    if (strcmp(mp->nm_modname, name) == 0) break;
  }

  CHECK(mp == nullptr || (mp->nm_flags & flag) != 0);
  return mp;
}

static Local<Object> InitModule(Environment* env,
                                pure_module* mod,
                                Local<String> module) {
  // Internal bindings don't have a "module" object, only exports.
  Local<Function> ctor = env->binding_data_ctor_template()
                             ->GetFunction(env->context())
                             .ToLocalChecked();
  // TODO
  // 为什么这里不需要使用 EscapableHandleScopes ?
  // 暂且认为是 nm_context_register_func 存取引用了 exports
  Local<Object> exports = ctor->NewInstance(env->context()).ToLocalChecked();
  CHECK_NULL(mod->nm_register_func);
  CHECK_NOT_NULL(mod->nm_context_register_func);
  Local<Value> unused = Undefined(env->isolate());
  mod->nm_context_register_func(exports, unused, env->context(), mod->nm_priv);
  return exports;
}

MaybeLocal<Object> GetInternalBinding(Environment* env, const char* module_v) {
  Local<Object> exports;
  Local<String> module;
  pure_module* mod = FindModule(modlist_internal, module_v, NM_F_INTERNAL);
  if (mod != nullptr) {
    exports = InitModule(env, mod, module);
    // env->internal_bindings.insert(mod);
    return exports;
  }
  return MaybeLocal<Object>();
}

void GetInternalBinding(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);

  CHECK(args[0]->IsString());

  Local<String> module = args[0].As<String>();
  pure::Utf8Value module_v(env->isolate(), module);
  Local<Object> exports;

  pure_module* mod = FindModule(modlist_internal, *module_v, NM_F_INTERNAL);
  if (mod != nullptr) {
    exports = InitModule(env, mod, module);
    // env->internal_bindings.insert(mod);
  }
  // else if (!strcmp(*module_v, "constants"))
  // {
  //     exports = Object::New(env->isolate());
  //     CHECK(
  //         exports->SetPrototype(env->context(),
  //         Null(env->isolate())).FromJust());
  //     DefineConstants(env->isolate(), exports);
  // }
  // else if (!strcmp(*module_v, "natives"))
  // {
  //     exports =
  //     native_module::NativeModuleEnv::GetSourceObject(env->context());
  //     // Legacy feature: process.binding('natives').config contains
  //     stringified
  //     // config.gypi
  //     CHECK(exports
  //               ->Set(env->context(),
  //                     env->config_string(),
  //                     native_module::NativeModuleEnv::GetConfigString(
  //                         env->isolate()))
  //               .FromJust());
  // }
  else {
    // char errmsg[1024];
    // snprintf(errmsg, sizeof(errmsg), "No such module: %s", *module_v);
    // return THROW_ERR_INVALID_MODULE(env, errmsg);
  }

  args.GetReturnValue().Set(exports);
}

void RegisterBuiltinModules() {
#define V(modname) _register_##modname();
  PURE_BUILTIN_MODULES(V);
#undef V
}
}  // namespace binding

}  // namespace pure
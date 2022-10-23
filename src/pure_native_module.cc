#include <string.h>
#include <algorithm>
#include <iostream>

#include "env-inl.h"
#include "pure_native_module.h"
#include "util-inl.h"

#include "pure_mutex.h"

namespace pure {
using v8::Context;
using v8::EscapableHandleScope;
using v8::Function;
using v8::IntegrityLevel;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::Object;
using v8::ScriptCompiler;
using v8::ScriptOrigin;
using v8::String;
using v8::Value;

int ReadFileSync(std::string* result, const char* path) {
  uv_fs_t req;
  auto defer_req_cleanup = OnScopeLeave([&req]() { uv_fs_req_cleanup(&req); });

  uv_file file = uv_fs_open(nullptr, &req, path, O_RDONLY, 0, nullptr);
  if (req.result < 0) {
    // req will be cleaned up by scope leave.
    return req.result;
  }
  uv_fs_req_cleanup(&req);

  auto defer_close = OnScopeLeave([file]() {
    uv_fs_t close_req;
    CHECK_EQ(0, uv_fs_close(nullptr, &close_req, file, nullptr));
    uv_fs_req_cleanup(&close_req);
  });

  *result = std::string("");
  char buffer[4096];
  uv_buf_t buf = uv_buf_init(buffer, sizeof(buffer));

  while (true) {
    const int r =
        uv_fs_read(nullptr, &req, file, &buf, 1, result->length(), nullptr);
    if (req.result < 0) {
      // req will be cleaned up by scope leave.
      return req.result;
    }
    uv_fs_req_cleanup(&req);
    if (r <= 0) {
      break;
    }
    result->append(buf.base, r);
  }
  return 0;
}

namespace native_module {
NativeModuleLoader NativeModuleLoader::instance_;

NativeModuleLoader::NativeModuleLoader() {
  LoadJavaScriptSource();
}

NativeModuleLoader* NativeModuleLoader::GetInstance() {
  return &instance_;
}

MaybeLocal<String> NativeModuleLoader::LoadBuiltinModuleSource(Isolate* isolate,
                                                               const char* id) {
  std::string filename = std::string(id);

  const auto source_it = source_.find(filename);

  if (UNLIKELY(source_it == source_.end())) {
    fprintf(stderr, "Cannot find native builtin: \"%s\".\n", id);
    abort();
  }

  return source_it->second.ToStringChecked(isolate);
}

MaybeLocal<String> NativeModuleLoader::LoadUserModuleSource(Isolate* isolate,
                                                            const char* id) {
  std::string filename = std::string(id);

  std::string contents;
  int r = ReadFileSync(&contents, filename.c_str());
  if (r != 0) {
    // std::cout << "ReadFileSync failed."
    //           << "\n";

    // const std::string buf = SPrintF("Cannot read local builtin. %s: %s
    // \"%s\"",
    //                                 uv_err_name(r),
    //                                 uv_strerror(r),
    //                                 filename);
    // Local<String> message = OneByteString(isolate, buf.c_str());
    // isolate->ThrowException(v8::Exception::Error(message));
    return MaybeLocal<String>();
  }
  return String::NewFromUtf8(
      isolate, contents.c_str(), v8::NewStringType::kNormal, contents.length());
}

// Returns Local<Function> of the compiled module if return_code_cache
// is false (we are only compiling the function).
// Otherwise return a Local<Object> containing the cache.
MaybeLocal<Function> NativeModuleLoader::LookupAndCompile(
    Local<Context> context,
    const char* id,
    std::vector<Local<String>>* parameters,
    Environment* optional_env) {
  Isolate* isolate = context->GetIsolate();
  EscapableHandleScope scope(isolate);
  Local<String> source;

  if (strncmp(id, "pure:", strlen("pure:")) == 0) {
    if (!LoadBuiltinModuleSource(isolate, id).ToLocal(&source)) {
      std::cout << "LoadBuiltinModuleSource failed."
                << "\n";
      return {};
    }

    // DEBUG
    // pure::Utf8Value source2(isolate, source);
    // std::cout << *source2 << 'end\n';
  } else if (!LoadUserModuleSource(isolate, id).ToLocal(&source)) {
    std::cout << "LoadUserModuleSource failed."
              << "\n";
    return {};
  }

  std::string filename_s = std::string("pure:") + id;
  Local<String> filename =
      OneByteString(isolate, filename_s.c_str(), filename_s.size());
  ScriptOrigin origin(isolate, filename, 0, 0, true);

  ScriptCompiler::CompileOptions options = ScriptCompiler::kEagerCompile;
  ScriptCompiler::Source script_source(source, origin, nullptr);

  v8::Local<v8::Script> script =
      v8::Script::Compile(context, source).ToLocalChecked();

  v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

  v8::String::Utf8Value utf8(isolate, result);

  // MaybeLocal<Function> maybe_fun =
  //     ScriptCompiler::CompileFunctionInContext(context,
  //                                              &script_source,
  //                                              parameters->size(),
  //                                              parameters->data(),
  //                                              0,
  //                                              nullptr,
  //                                              options);

  // // This could fail when there are early errors in the native modules,
  // // e.g. the syntax errors
  // Local<Function> fun;
  // if (!maybe_fun.ToLocal(&fun))
  // {
  //     // In the case of early errors, v8 is already capable of
  //     // decorating the stack for us - note that we use
  //     CompileFunctionInContext
  //     // so there is no need to worry about wrappers.
  //     return MaybeLocal<Function>();
  // }
  // return scope.Escape(fun);
  return MaybeLocal<Function>();
}
}  // namespace native_module
}  // namespace pure
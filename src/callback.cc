#include "env-inl.h"
#include "pure.h"
#include "v8.h"

namespace pure {
using v8::Context;
using v8::Function;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::Object;
using v8::String;
using v8::Value;

CallbackScope::CallbackScope(Isolate* isolate, Local<Object> object)
    : CallbackScope(Environment::GetCurrent(isolate), object) {}

CallbackScope::CallbackScope(Environment* env, Local<Object> object)
    : private_(new InternalCallbackScope(env, object)),
      try_catch_(env->isolate()) {
  try_catch_.SetVerbose(true);
}

CallbackScope::~CallbackScope() {
  if (try_catch_.HasCaught()) private_->MarkAsFailed();
  delete private_;
}

// InternalCallbackScope isolate->SetIdle(false);
// ~InternalCallbackScope isolate->SetIdle(true);
InternalCallbackScope::InternalCallbackScope(Environment* env,
                                             Local<Object> object)
    : env_(env), object_(object) {
  CHECK_NOT_NULL(env);

  if (!env->can_call_into_js()) {
    failed_ = true;
    return;
  }

  Isolate* isolate = env->isolate();

  HandleScope handle_scope(isolate);
  Local<Context> current_context = isolate->GetCurrentContext();
  if (UNLIKELY(env->context() != current_context)) {
    CHECK_EQ(Environment::GetCurrent(isolate), env);
  }

  isolate->SetIdle(false);
}

InternalCallbackScope::~InternalCallbackScope() {
  Close();
}

void InternalCallbackScope::Close() {
  if (closed_) return;
  closed_ = true;

  Isolate* isolate = env_->isolate();
  auto idle = OnScopeLeave([&]() { isolate->SetIdle(true); });

  if (!env_->can_call_into_js()) return;
  auto perform_stopping_check = [&]() {
    if (env_->is_stopping()) {
      MarkAsFailed();
    }
  };
  perform_stopping_check();

  if (failed_) return;

  //   TickInfo* tick_info = env_->tick_info();

  if (!env_->can_call_into_js()) return;

  //   auto weakref_cleanup = OnScopeLeave([&]() { env_->RunWeakRefCleanup();
  //   });
}

MaybeLocal<Value> InternalMakeCallback(Environment* env,
                                       Local<Object> resource,
                                       Local<Object> recv,
                                       const Local<Function> callback,
                                       int argc,
                                       Local<Value> argv[]) {
  CHECK(!recv.IsEmpty());

  InternalCallbackScope scope(env, resource);
  if (scope.Failed()) {
    return MaybeLocal<Value>();
  }

  MaybeLocal<Value> ret;

  Local<Context> context = env->context();
  ret = callback->Call(context, recv, argc, argv);

  if (ret.IsEmpty()) {
    scope.MarkAsFailed();
    return MaybeLocal<Value>();
  }

  scope.Close();
  if (scope.Failed()) {
    return MaybeLocal<Value>();
  }

  return ret;
}

// Public MakeCallback()s

MaybeLocal<Value> MakeCallback(Isolate* isolate,
                               Local<Object> recv,
                               const char* method,
                               int argc,
                               Local<Value> argv[]) {
  Local<String> method_string =
      String::NewFromUtf8(isolate, method).ToLocalChecked();
  return MakeCallback(isolate, recv, method_string, argc, argv);
}

MaybeLocal<Value> MakeCallback(Isolate* isolate,
                               Local<Object> recv,
                               Local<String> symbol,
                               int argc,
                               Local<Value> argv[]) {
  // Check can_call_into_js() first because calling Get() might do so.
  Environment* env =
      Environment::GetCurrent(recv->GetCreationContext().ToLocalChecked());
  CHECK_NOT_NULL(env);
  if (!env->can_call_into_js()) return Local<Value>();

  Local<Value> callback_v;
  if (!recv->Get(isolate->GetCurrentContext(), symbol).ToLocal(&callback_v))
    return Local<Value>();
  if (!callback_v->IsFunction()) {
    // This used to return an empty value, but Undefined() makes more sense
    // since no exception is pending here.
    return Undefined(isolate);
  }
  Local<Function> callback = callback_v.As<Function>();
  return MakeCallback(isolate, recv, callback, argc, argv);
}

MaybeLocal<Value> MakeCallback(Isolate* isolate,
                               Local<Object> recv,
                               Local<Function> callback,
                               int argc,
                               Local<Value> argv[]) {
  Environment* env =
      Environment::GetCurrent(callback->GetCreationContext().ToLocalChecked());
  CHECK_NOT_NULL(env);
  Context::Scope context_scope(env->context());
  MaybeLocal<Value> ret =
      InternalMakeCallback(env, recv, recv, callback, argc, argv);
  return ret;
}
}  // namespace pure
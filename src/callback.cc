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

InternalCallbackScope::InternalCallbackScope(Environment* env,
                                             Local<Object> object)
    : env_(env), object_(object) {
  CHECK_NOT_NULL(env);
  //   env->PushAsyncCallbackScope();

  if (!env->can_call_into_js()) {
    failed_ = true;
    return;
  }

  Isolate* isolate = env->isolate();

  HandleScope handle_scope(isolate);
  Local<Context> current_context = isolate->GetCurrentContext();
  // If you hit this assertion, the caller forgot to enter the right Node.js
  // Environment's v8::Context first.
  // We first check `env->context() != current_context` because the contexts
  // likely *are* the same, in which case we can skip the slightly more
  // expensive Environment::GetCurrent() call.
  if (UNLIKELY(env->context() != current_context)) {
    CHECK_EQ(Environment::GetCurrent(isolate), env);
  }

  isolate->SetIdle(false);

  //   env->async_hooks()->push_async_context(
  //       async_context_.async_id, async_context_.trigger_async_id, object);

  // pushed_ids_ = true;

  //   if (asyncContext.async_id != 0 && !skip_hooks_) {
  //     // No need to check a return value because the application will exit if
  //     // an exception occurs.
  //     AsyncWrap::EmitBefore(env, asyncContext.async_id);
  //   }
}

InternalCallbackScope::~InternalCallbackScope() {
  Close();
  //   env_->PopAsyncCallbackScope();
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
  // Observe the following two subtleties:
  //
  // 1. The environment is retrieved from the callback function's context.
  // 2. The context to enter is retrieved from the environment.
  //
  // Because of the AssignToContext() call in src/node_contextify.cc,
  // the two contexts need not be the same.
  Environment* env =
      Environment::GetCurrent(callback->GetCreationContext().ToLocalChecked());
  CHECK_NOT_NULL(env);
  Context::Scope context_scope(env->context());
  MaybeLocal<Value> ret =
      InternalMakeCallback(env, recv, recv, callback, argc, argv);
  //   if (ret.IsEmpty() && env->async_callback_scope_depth() == 0) {
  //     // This is only for legacy compatibility and we may want to look into
  //     // removing/adjusting it.
  //     return Undefined(isolate);
  //   }
  return ret;
}
}  // namespace pure
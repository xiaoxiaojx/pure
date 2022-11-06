#include "pure_errors.h"
#include "env-inl.h"
#include "env.h"
#include "pure_mutex.h"
#include "pure_options.h"
#include "util.h"
#include "v8.h"
#include "debug_utils.h"

#include <sstream>

namespace pure {
using errors::TryCatchScope;
using v8::Boolean;
using v8::Context;
using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::HandleScope;
using v8::Int32;
using v8::Isolate;
using v8::Just;
using v8::Local;
using v8::Maybe;
using v8::MaybeLocal;
using v8::Message;
using v8::Object;
using v8::ScriptOrigin;
using v8::StackFrame;
using v8::StackTrace;
using v8::String;
using v8::Undefined;
using v8::Value;

namespace errors {

void PerIsolateMessageListener(Local<Message> message, Local<Value> error) {
  Isolate* isolate = message->GetIsolate();
  switch (message->ErrorLevel()) {
    case Isolate::MessageErrorLevel::kMessageWarning: {
      Environment* env = Environment::GetCurrent(isolate);
      if (!env) {
        break;
      }
      Utf8Value filename(isolate, message->GetScriptOrigin().ResourceName());
      // (filename):(line) (message)
      std::stringstream warning;
      warning << *filename;
      warning << ":";
      warning << message->GetLineNumber(env->context()).FromMaybe(-1);
      warning << " ";
      v8::String::Utf8Value msg(isolate, message->Get());
      warning << *msg;
      // USE(ProcessEmitWarningGeneric(env, warning.str().c_str(), "V8"));
      break;
    }
    case Isolate::MessageErrorLevel::kMessageError:
      // TriggerUncaughtException(isolate, error, message);
      break;
  }
}

void OnFatalError(const char* location, const char* message) {
  if (location)
   {
       FPrintF(stderr, "FATAL ERROR: %s %s\n", location, message);
   }
   else
   {
       FPrintF(stderr, "FATAL ERROR: %s\n", message);
   }

  Isolate* isolate = Isolate::TryGetCurrent();
  Environment* env = nullptr;
  if (isolate != nullptr) {
    env = Environment::GetCurrent(isolate);
  }
  bool report_on_fatalerror;
  {
    Mutex::ScopedLock lock(pure::per_process::cli_options_mutex);
    report_on_fatalerror = per_process::cli_options->report_on_fatalerror;
  }

  if (report_on_fatalerror) {
    // TODO
    // report::TriggerNodeReport(
    //     isolate, env, message, "FatalError", "", Local<Object>());
  }

  fflush(stderr);
  abort();
}

enum class EnhanceFatalException { kEnhance, kDontEnhance };

static void ReportFatalException(Environment* env,
                                 Local<Value> error,
                                 Local<Message> message,
                                 EnhanceFatalException enhance_stack) {
  if (!env->can_call_into_js())
    enhance_stack = EnhanceFatalException::kDontEnhance;

  // TODO
}

TryCatchScope::~TryCatchScope() {
  if (HasCaught() && !HasTerminated() && mode_ == CatchMode::kFatal) {
    HandleScope scope(env_->isolate());
    Local<v8::Value> exception = Exception();
    Local<v8::Message> message = Message();
    EnhanceFatalException enhance = CanContinue()
                                        ? EnhanceFatalException::kEnhance
                                        : EnhanceFatalException::kDontEnhance;
    if (message.IsEmpty())
      message = Exception::CreateMessage(env_->isolate(), exception);
    ReportFatalException(env_, exception, message, enhance);
    env_->Exit(7);
  }
}

void TriggerUncaughtException(Isolate* isolate,
                              Local<Value> error,
                              Local<Message> message,
                              bool from_promise) {
  CHECK(!error.IsEmpty());
  HandleScope scope(isolate);

  if (message.IsEmpty()) message = Exception::CreateMessage(isolate, error);

  CHECK(isolate->InContext());
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  if (env == nullptr) {
    Abort();
  }

  // TODO 这里调用用户注册的 errorHandle

  // Now we are certain that the exception is fatal.
  ReportFatalException(env, error, message, EnhanceFatalException::kEnhance);
  RunAtExit(env);

  env->Exit(1);
}

void TriggerUncaughtException(Isolate* isolate, const v8::TryCatch& try_catch) {
  if (try_catch.IsVerbose()) {
    return;
  }

  CHECK(!try_catch.HasTerminated());
  CHECK(try_catch.HasCaught());
  HandleScope scope(isolate);
  TriggerUncaughtException(isolate,
                           try_catch.Exception(),
                           try_catch.Message(),
                           false /* from_promise */);
}

}  // namespace errors
}  // namespace pure
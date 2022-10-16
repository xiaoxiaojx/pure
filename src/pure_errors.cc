#include "v8.h"
#include "env-inl.h"
#include "env.h"

#include "util.h"

#include <sstream> 

namespace pure
{
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

    namespace errors
    {
        void PerIsolateMessageListener(Local<Message> message, Local<Value> error)
        {
            Isolate *isolate = message->GetIsolate();
            switch (message->ErrorLevel())
            {
            case Isolate::MessageErrorLevel::kMessageWarning:
            {
                Environment *env = Environment::GetCurrent(isolate);
                if (!env)
                {
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

    }
}
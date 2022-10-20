#include "v8.h"
#include "env-inl.h"
#include "env.h"
#include "pure_mutex.h"
#include "pure_options.h"
#include "util.h"

#include <sstream>

namespace pure
{
    using v8::Context;
    using v8::Exception;
    using v8::HandleScope;
    using v8::Isolate;
    using v8::Local;
    using v8::Maybe;
    using v8::Message;
    using v8::Object;
    using v8::ScriptOrigin;
    using v8::String;
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

        void OnFatalError(const char *location, const char *message)
        {
            // if (location)
            //  {
            //      FPrintF(stderr, "FATAL ERROR: %s %s\n", location, message);
            //  }
            //  else
            //  {
            //      FPrintF(stderr, "FATAL ERROR: %s\n", message);
            //  }

            Isolate *isolate = Isolate::TryGetCurrent();
            Environment *env = nullptr;
            if (isolate != nullptr)
            {
                env = Environment::GetCurrent(isolate);
            }
            bool report_on_fatalerror;
            {
                Mutex::ScopedLock lock(pure::per_process::cli_options_mutex);
                report_on_fatalerror = per_process::cli_options->report_on_fatalerror;
            }

            if (report_on_fatalerror)
            {
                // TODO
                // report::TriggerNodeReport(
                //     isolate, env, message, "FatalError", "", Local<Object>());
            }

            fflush(stderr);
            abort();
        }

    }
}
#include "util.h"
#include "v8.h"
#include "uv.h"
#include "pure.h"

namespace pure
{

    using v8::ArrayBufferView;
    using v8::Isolate;
    using v8::Local;
    using v8::String;
    using v8::Value;

    template <typename T>
    static void MakeUtf8String(Isolate *isolate,
                               Local<Value> value,
                               MaybeStackBuffer<T> *target)
    {
        Local<String> string;
        if (!value->ToString(isolate->GetCurrentContext()).ToLocal(&string))
            return;

        size_t storage;
        // if (!StringBytes::StorageSize(isolate, string, UTF8).To(&storage))
        //     return;
        storage += 1;
        target->AllocateSufficientStorage(storage);
        const int flags =
            String::NO_NULL_TERMINATION | String::REPLACE_INVALID_UTF8;
        const int length =
            string->WriteUtf8(isolate, target->out(), storage, nullptr, flags);
        target->SetLengthAndZeroTerminate(length);
    }

    Utf8Value::Utf8Value(Isolate *isolate, Local<Value> value)
    {
        if (value.IsEmpty())
            return;

        MakeUtf8String(isolate, value, this);
    }
}
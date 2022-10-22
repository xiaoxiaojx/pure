#include <cmath>
#include <iostream>

#include "util.h"
#include "v8.h"
#include "uv.h"
#include "pure.h"

namespace pure
{

    using v8::Isolate;
    using v8::Local;
    using v8::String;
    using v8::Value;

    template <typename T>
    static void MakeUtf8String(Isolate *isolate,
                               Local<String> string,
                               MaybeStackBuffer<T> *target)
    {
        size_t storage;
        // TODO
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

    template <typename T>
    static void MakeUtf8String(Isolate *isolate,
                               Local<Value> value,
                               MaybeStackBuffer<T> *target)
    {
        Local<String> string;
        if (!value->ToString(isolate->GetCurrentContext()).ToLocal(&string))
            return;

        MakeUtf8String(isolate, string, target);
    }

    Utf8Value::Utf8Value(Isolate *isolate, Local<Value> value)
    {
        if (value.IsEmpty())
            return;

        MakeUtf8String(isolate, value, this);
    }

    Utf8Value::Utf8Value(Isolate *isolate, Local<String> value)
    {
        if (value.IsEmpty())
            return;

        MakeUtf8String(isolate, value, this);
    }

    [[noreturn]] void Abort()
    {
        std::cout << ">> abort\n";

        std::fflush(stderr);
        std::abort();
    }

    [[noreturn]] void Assert(const AssertionInfo &info)
    {
        std::string location =
            std::string(info.file_line) + ":" + std::string(info.function);
        std::string message =
            "Assertion `" + std::string(info.message) + "' failed.\n";

        std::cout << message << ">>> assert\n";

        // TODO
        // OnFatalError(location.c_str(), message.c_str());
        std::abort();
    }

    void LowMemoryNotification()
    {
        if (per_process::v8_initialized)
        {
            auto isolate = Isolate::TryGetCurrent();
            if (isolate != nullptr)
            {
                isolate->LowMemoryNotification();
            }
        }
    }

    // Catch and compute overflow during multiplication of two large integers https://stackoverflow.com/questions/1815367/catch-and-compute-overflow-during-multiplication-of-two-large-integers
    // 简单的检测 a * b 是否因为结果过大而溢出
    template <typename T>
    inline T MultiplyWithOverflowCheck(T a, T b)
    {
        auto ret = a * b;
        if (a != 0)
            CHECK_EQ(b, ret / a);

        return ret;
    }

    // Used by the allocation functions when allocation fails.
    // Thin wrapper around v8::Isolate::LowMemoryNotification() that checks
    // whether V8 is initialized.
    void LowMemoryNotification();

    // These should be used in our code as opposed to the native
    // versions as they abstract out some platform and or
    // compiler version specific functionality.
    // malloc(0) and realloc(ptr, 0) have implementation-defined behavior in
    // that the standard allows them to either return a unique pointer or a
    // nullptr for zero-sized allocation requests.  Normalize by always using
    // a nullptr.
    template <typename T>
    T *UncheckedRealloc(T *pointer, size_t n)
    {
        size_t full_size = MultiplyWithOverflowCheck(sizeof(T), n);

        if (full_size == 0)
        {
            free(pointer);
            return nullptr;
        }

        void *allocated = realloc(pointer, full_size);

        if (UNLIKELY(allocated == nullptr))
        {
            // Tell V8 that memory is low and retry.
            LowMemoryNotification();
            allocated = realloc(pointer, full_size);
        }

        return static_cast<T *>(allocated);
    }

    // As per spec realloc behaves like malloc if passed nullptr.
    template <typename T>
    inline T *UncheckedMalloc(size_t n)
    {
        if (n == 0)
            n = 1;
        return UncheckedRealloc<T>(nullptr, n);
    }

    template <typename T>
    inline T *UncheckedCalloc(size_t n)
    {
        if (n == 0)
            n = 1;
        MultiplyWithOverflowCheck(sizeof(T), n);
        return static_cast<T *>(calloc(n, sizeof(T)));
    }

    template <typename T>
    inline T *Realloc(T *pointer, size_t n)
    {
        T *ret = UncheckedRealloc(pointer, n);
        CHECK_IMPLIES(n > 0, ret != nullptr);
        return ret;
    }

    template <typename T>
    inline T *Malloc(size_t n)
    {
        T *ret = UncheckedMalloc<T>(n);
        CHECK_IMPLIES(n > 0, ret != nullptr);
        return ret;
    }

    template <typename T>
    inline T *Calloc(size_t n)
    {
        T *ret = UncheckedCalloc<T>(n);
        CHECK_IMPLIES(n > 0, ret != nullptr);
        return ret;
    }
}
#ifndef SRC_UTIL_INL_H_
#define SRC_UTIL_INL_H_

#include "v8.h"
#include "uv.h"
#include "util.h"

namespace pure
{

#ifdef __GNUC__
#define MUST_USE_RESULT __attribute__((warn_unused_result))
#else
#define MUST_USE_RESULT
#endif

    inline v8::Local<v8::String> OneByteString(v8::Isolate *isolate,
                                               const char *data,
                                               int length)
    {
        return v8::String::NewFromOneByte(isolate,
                                          reinterpret_cast<const uint8_t *>(data),
                                          v8::NewStringType::kNormal,
                                          length)
            .ToLocalChecked();
    }

    inline v8::Local<v8::String> OneByteString(v8::Isolate *isolate,
                                               const signed char *data,
                                               int length)
    {
        return v8::String::NewFromOneByte(isolate,
                                          reinterpret_cast<const uint8_t *>(data),
                                          v8::NewStringType::kNormal,
                                          length)
            .ToLocalChecked();
    }

    inline v8::Local<v8::String> OneByteString(v8::Isolate *isolate,
                                               const unsigned char *data,
                                               int length)
    {
        return v8::String::NewFromOneByte(
                   isolate, data, v8::NewStringType::kNormal, length)
            .ToLocalChecked();
    }
    // Used to be a macro, hence the uppercase name.
    template <int N>
    inline v8::Local<v8::String> FIXED_ONE_BYTE_STRING(
        v8::Isolate *isolate,
        const char (&data)[N])
    {
        return OneByteString(isolate, data, N - 1);
    }

    template <std::size_t N>
    inline v8::Local<v8::String> FIXED_ONE_BYTE_STRING(
        v8::Isolate *isolate,
        const std::array<char, N> &arr)
    {
        return OneByteString(isolate, arr.data(), N - 1);
    }

    // Use this when a variable or parameter is unused in order to explicitly
    // silence a compiler warning about that.
    template <typename T>
    inline void USE(T &&) {}

    template <typename Fn>
    struct OnScopeLeaveImpl
    {
        Fn fn_;
        bool active_;

        explicit OnScopeLeaveImpl(Fn &&fn) : fn_(std::move(fn)), active_(true) {}
        ~OnScopeLeaveImpl()
        {
            if (active_)
                fn_();
        }

        OnScopeLeaveImpl(const OnScopeLeaveImpl &other) = delete;
        OnScopeLeaveImpl &operator=(const OnScopeLeaveImpl &other) = delete;
        OnScopeLeaveImpl(OnScopeLeaveImpl &&other)
            : fn_(std::move(other.fn_)), active_(other.active_)
        {
            other.active_ = false;
        }
        OnScopeLeaveImpl &operator=(OnScopeLeaveImpl &&other)
        {
            if (this == &other)
                return *this;
            this->~OnScopeLeave();
            new (this) OnScopeLeaveImpl(std::move(other));
            return *this;
        }
    };

    // Run a function when exiting the current scope. Used like this:
    // auto on_scope_leave = OnScopeLeave([&] {
    //   // ... run some code ...
    // });
    template <typename Fn>
    inline MUST_USE_RESULT OnScopeLeaveImpl<Fn> OnScopeLeave(Fn &&fn)
    {
        return OnScopeLeaveImpl<Fn>{std::move(fn)};
    }
}

#endif // SRC_UTIL_INL_H_
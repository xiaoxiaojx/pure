#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_

#include "assert.h"
#include <memory>

#include "v8.h"

namespace pure

{

    template <typename T, size_t N>
    constexpr size_t arraysize(const T (&)[N])
    {
        return N;
    }

    template <typename T, size_t N>
    constexpr size_t strsize(const T (&)[N])
    {
        return N - 1;
    }
    template <typename T, size_t kStackStorageSize = 1024>
    class MaybeStackBuffer
    {
    public:
        const T *out() const
        {
            return buf_;
        }

        T *out()
        {
            return buf_;
        }

        // operator* for compatibility with `v8::String::(Utf8)Value`
        T *operator*()
        {
            return buf_;
        }

        const T *operator*() const
        {
            return buf_;
        }

        T &operator[](size_t index)
        {
            CHECK_LT(index, length());
            return buf_[index];
        }

        const T &operator[](size_t index) const
        {
            CHECK_LT(index, length());
            return buf_[index];
        }

        size_t length() const
        {
            return length_;
        }

        // Current maximum capacity of the buffer with which SetLength() can be used
        // without first calling AllocateSufficientStorage().
        size_t capacity() const
        {
            return capacity_;
        }

        // Make sure enough space for `storage` entries is available.
        // This method can be called multiple times throughout the lifetime of the
        // buffer, but once this has been called Invalidate() cannot be used.
        // Content of the buffer in the range [0, length()) is preserved.
        void AllocateSufficientStorage(size_t storage)
        {
            assert(!IsInvalidated());
            if (storage > capacity())
            {
                // TODO
                // bool was_allocated = IsAllocated();
                // T *allocated_ptr = was_allocated ? buf_ : nullptr;
                // buf_ = realloc((char *)allocated_ptr, storage);
                // capacity_ = storage;
                // if (!was_allocated && length_ > 0)
                //     memcpy(buf_, buf_st_, length_ * sizeof(buf_[0]));
            }

            length_ = storage;
        }

        void SetLength(size_t length)
        {
            // capacity() returns how much memory is actually available.
            // CHECK_LE(length, capacity());
            length_ = length;
        }

        void SetLengthAndZeroTerminate(size_t length)
        {
            // capacity() returns how much memory is actually available.
            // CHECK_LE(length + 1, capacity());
            SetLength(length);

            // T() is 0 for integer types, nullptr for pointers, etc.
            buf_[length] = T();
        }

        // Make dereferencing this object return nullptr.
        // This method can be called multiple times throughout the lifetime of the
        // buffer, but once this has been called AllocateSufficientStorage() cannot
        // be used.
        void Invalidate()
        {
            CHECK(!IsAllocated());
            capacity_ = 0;
            length_ = 0;
            buf_ = nullptr;
        }

        // If the buffer is stored in the heap rather than on the stack.
        bool IsAllocated() const
        {
            return !IsInvalidated() && buf_ != buf_st_;
        }

        // If Invalidate() has been called.
        bool IsInvalidated() const
        {
            return buf_ == nullptr;
        }

        // Release ownership of the malloc'd buffer.
        // Note: This does not free the buffer.
        void Release()
        {
            CHECK(IsAllocated());
            buf_ = buf_st_;
            length_ = 0;
            capacity_ = arraysize(buf_st_);
        }

        MaybeStackBuffer()
            : length_(0), capacity_(arraysize(buf_st_)), buf_(buf_st_)
        {
            // Default to a zero-length, null-terminated buffer.
            buf_[0] = T();
        }

        explicit MaybeStackBuffer(size_t storage) : MaybeStackBuffer()
        {
            AllocateSufficientStorage(storage);
        }

        ~MaybeStackBuffer()
        {
            if (IsAllocated())
                free(buf_);
        }

    private:
        size_t length_;
        // capacity of the malloc'ed buf_
        size_t capacity_;
        T *buf_;
        T buf_st_[kStackStorageSize];
    };

    class Utf8Value : public MaybeStackBuffer<char>
    {
    public:
        explicit Utf8Value(v8::Isolate *isolate, v8::Local<v8::Value> value);

        inline std::string ToString() const { return std::string(out(), length()); }

        inline bool operator==(const char *a) const
        {
            return strcmp(out(), a) == 0;
        }
    };

    // Convert a v8::PersistentBase, e.g. v8::Global, to a Local, with an extra
    // optimization for strong persistent handles.
    class PersistentToLocal
    {
    public:
        // If persistent.IsWeak() == false, then do not call persistent.Reset()
        // while the returned Local<T> is still in scope, it will destroy the
        // reference to the object.
        template <class TypeName>
        static inline v8::Local<TypeName> Default(
            v8::Isolate *isolate,
            const v8::PersistentBase<TypeName> &persistent)
        {
            if (persistent.IsWeak())
            {
                return PersistentToLocal::Weak(isolate, persistent);
            }
            else
            {
                return PersistentToLocal::Strong(persistent);
            }
        }

        // Unchecked conversion from a non-weak Persistent<T> to Local<T>,
        // use with care!
        //
        // Do not call persistent.Reset() while the returned Local<T> is still in
        // scope, it will destroy the reference to the object.
        template <class TypeName>
        static inline v8::Local<TypeName> Strong(
            const v8::PersistentBase<TypeName> &persistent)
        {
            // DCHECK(!persistent.IsWeak());
            return *reinterpret_cast<v8::Local<TypeName> *>(
                const_cast<v8::PersistentBase<TypeName> *>(&persistent));
        }

        template <class TypeName>
        static inline v8::Local<TypeName> Weak(
            v8::Isolate *isolate,
            const v8::PersistentBase<TypeName> &persistent)
        {
            return v8::Local<TypeName>::New(isolate, persistent);
        }
    };
}

#endif // SRC_UTIL_H_

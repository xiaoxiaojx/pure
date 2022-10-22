#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_

#include "assert.h"
#include <memory>

#include "v8.h"

namespace pure

{

    namespace per_process
    {
        // Tells whether the per-process V8::Initialize() is called and
        // if it is safe to call v8::Isolate::TryGetCurrent().
        extern bool v8_initialized;

        extern std::unique_ptr<v8::Platform> v8_platform;

    }

    // Convenience wrapper around v8::String::NewFromOneByte().
    inline v8::Local<v8::String> OneByteString(v8::Isolate *isolate,
                                               const char *data,
                                               int length = -1);

    // For the people that compile with -funsigned-char.
    inline v8::Local<v8::String> OneByteString(v8::Isolate *isolate,
                                               const signed char *data,
                                               int length = -1);

    inline v8::Local<v8::String> OneByteString(v8::Isolate *isolate,
                                               const unsigned char *data,
                                               int length = -1);

    template <typename T, void (*function)(T *)>
    struct FunctionDeleter
    {
        // operator() 操作符让一个结构体能够像一个函数一样被调用
        // 类似于 js 中的函数, 也是对象
        void operator()(T *pointer) const { function(pointer); }
        typedef std::unique_ptr<T, FunctionDeleter> Pointer;
    };

    template <typename T, void (*function)(T *)>
    using DeleteFnPtr = typename FunctionDeleter<T, function>::Pointer;

    // The reason that Assert() takes a struct argument instead of individual
    // const char*s is to ease instruction cache pressure in calls from CHECK.
    struct AssertionInfo
    {
        const char *file_line; // filename:line
        const char *message;
        const char *function;
    };
    [[noreturn]] void Abort();
    [[noreturn]] void Assert(const AssertionInfo &info);

#define ERROR_AND_ABORT(expr)                                                     \
    do                                                                            \
    {                                                                             \
        /* Make sure that this struct does not end up in inline code, but      */ \
        /* rather in a read-only data section when modifying this code.        */ \
        static const pure::AssertionInfo args = {                                 \
            __FILE__ ":" STRINGIFY(__LINE__), #expr, PRETTY_FUNCTION_NAME};       \
        pure::Assert(args);                                                       \
    } while (0)

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#define CHECK(expr)                \
    do                             \
    {                              \
        if (UNLIKELY(!(expr)))     \
        {                          \
            ERROR_AND_ABORT(expr); \
        }                          \
    } while (0)

#define CHECK_EQ(a, b) CHECK((a) == (b))
#define CHECK_GE(a, b) CHECK((a) >= (b))
#define CHECK_GT(a, b) CHECK((a) > (b))
#define CHECK_LE(a, b) CHECK((a) <= (b))
#define CHECK_LT(a, b) CHECK((a) < (b))
#define CHECK_NE(a, b) CHECK((a) != (b))
#define CHECK_NULL(val) CHECK((val) == nullptr)
#define CHECK_NOT_NULL(val) CHECK((val) != nullptr)
#define CHECK_IMPLIES(a, b) CHECK(!(a) || (b))

#ifdef DEBUG
#define DCHECK(expr) CHECK(expr)
#define DCHECK_EQ(a, b) CHECK((a) == (b))
#define DCHECK_GE(a, b) CHECK((a) >= (b))
#define DCHECK_GT(a, b) CHECK((a) > (b))
#define DCHECK_LE(a, b) CHECK((a) <= (b))
#define DCHECK_LT(a, b) CHECK((a) < (b))
#define DCHECK_NE(a, b) CHECK((a) != (b))
#define DCHECK_NULL(val) CHECK((val) == nullptr)
#define DCHECK_NOT_NULL(val) CHECK((val) != nullptr)
#define DCHECK_IMPLIES(a, b) CHECK(!(a) || (b))
#else
#define DCHECK(expr)
#define DCHECK_EQ(a, b)
#define DCHECK_GE(a, b)
#define DCHECK_GT(a, b)
#define DCHECK_LE(a, b)
#define DCHECK_LT(a, b)
#define DCHECK_NE(a, b)
#define DCHECK_NULL(val)
#define DCHECK_NOT_NULL(val)
#define DCHECK_IMPLIES(a, b)
#endif

// __builtin_expect https://www.jianshu.com/p/2684613a300f
// __builtin_expect() 是 GCC (version >= 2.96）提供给程序员使用的，目的是将“分支转移”的信息提供给编译器，这样编译器可以对代码进行优化，以减少指令跳转带来的性能下降。
// __builtin_expect((x),1)表示 x 的值为真的可能性更大；
// __builtin_expect((x),0)表示 x 的值为假的可能性更大。
// 也就是说，使用likely()，执行 if 后面的语句的机会更大，使用 unlikely()，执行 else 后面的语句的机会更大。通过这种方式，编译器在编译过程中，会将可能性更大的代码紧跟着起面的代码，从而减少指令跳转带来的性能上的下降。
#ifdef __GNUC__
#define LIKELY(expr) __builtin_expect(!!(expr), 1)
#define UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#define PRETTY_FUNCTION_NAME __PRETTY_FUNCTION__
#else
#define LIKELY(expr) expr
#define UNLIKELY(expr) expr
#define PRETTY_FUNCTION_NAME ""
#endif

    // Same things, but aborts immediately instead of returning nullptr when
    // no memory is available.
    template <typename T>
    inline T *Realloc(T *pointer, size_t n);
    template <typename T>
    inline T *Malloc(size_t n);
    template <typename T>
    inline T *Calloc(size_t n);

    inline char *Malloc(size_t n);
    inline char *Calloc(size_t n);
    inline char *UncheckedMalloc(size_t n);
    inline char *UncheckedCalloc(size_t n);

    template <typename T>
    inline T MultiplyWithOverflowCheck(T a, T b);

    template <typename T, size_t N>
    constexpr size_t arraysize(const T (&)[N])
    {
        return N;
    }

    int ReadFileSync(std::string *result, const char *path);

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
            CHECK(!IsInvalidated());
            if (storage > capacity())
            {
                // TODO
                bool was_allocated = IsAllocated();
                T *allocated_ptr = was_allocated ? buf_ : nullptr;
                buf_ = Realloc(allocated_ptr, storage);
                capacity_ = storage;
                if (!was_allocated && length_ > 0)
                    memcpy(buf_, buf_st_, length_ * sizeof(buf_[0]));
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

        explicit Utf8Value(v8::Isolate *isolate, v8::Local<v8::String> value);

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

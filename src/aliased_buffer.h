#ifndef SRC_ALIASED_BUFFER_H_
#define SRC_ALIASED_BUFFER_H_

#include <cinttypes>
#include "iostream"
#include "util-inl.h"
#include "v8.h"
namespace pure {

typedef size_t AliasedBufferIndex;

// AliasedBufferBase
// 用于 JavaScript 二进制 Int32Array, Uint8Array 等与 C int32_t, uint8_ 等
// 数据交互的封装。 使得 C 和 JavaScript 中都可以 fields_[kRefCount] +=
// increment 这样去修改数据, 无需序列化转换操作, 因为指向的是同一块内存区域

// 通过如下接口即可让 JavaScript 直接操作
// target->Set(env->context(),
//               FIXED_ONE_BYTE_STRING(env->isolate(), "immediateInfo"),
//               env->immediate_info()->fields().GetJSArray()).Check();

// 类模版
template <class NativeT,
          class V8T,
          // is_scalar https://en.cppreference.com/w/cpp/types/is_scalar
          //           template<class T>
          // struct is_scalar : std::integral_constant<bool,
          //                      std::is_arithmetic<T>::value     ||
          //                      std::is_enum<T>::value           ||
          //                      std::is_pointer<T>::value        ||
          //                      std::is_member_pointer<T>::value ||
          //                      std::is_null_pointer<T>::value> {};
          // SFINAE NativeT to be scalar
          typename = std::enable_if_t<std::is_scalar<NativeT>::value>>
// SFINAE https://zhuanlan.zhihu.com/p/21314708
// 编译器不能识别如下两种 inc_counter, 会报 redefinition 错误, 于是有了 SFINAE
// template <typename T>
// void inc_counter(T& counterObj) {
//   counterObj.increase();
// }

// template <typename T>
// void inc_counter(T& intTypeCounter){
//   ++intTypeCounter;
// }

// void doSomething() {
//   Counter cntObj;
//   uint32_t cntUI32;

//   // blah blah blah
//   inc_counter(cntObj);
//   inc_counter(cntUI32);
// }
class AliasedBufferBase {
 public:
  AliasedBufferBase(v8::Isolate* isolate,
                    const size_t count,
                    const AliasedBufferIndex* index = nullptr)
      : isolate_(isolate), count_(count), byte_offset_(0), index_(index) {
    CHECK_GT(count, 0);
    if (index != nullptr) {
      // Will be deserialized later.
      return;
    }
    const v8::HandleScope handle_scope(isolate_);
    const size_t size_in_bytes =
        MultiplyWithOverflowCheck(sizeof(NativeT), count);

    // allocate v8 ArrayBuffer
    v8::Local<v8::ArrayBuffer> ab =
        v8::ArrayBuffer::New(isolate_, size_in_bytes);
    buffer_ = static_cast<NativeT*>(ab->GetBackingStore()->Data());

    // allocate v8 TypedArray
    v8::Local<V8T> js_array = V8T::New(ab, byte_offset_, count);
    js_array_ = v8::Global<V8T>(isolate, js_array);
  }

  /**
   * Create an AliasedBufferBase over a sub-region of another aliased buffer.
   * The two will share a v8::ArrayBuffer instance &
   * a native buffer, but will each read/write to different sections of the
   * native buffer.
   *
   *  Note that byte_offset must by aligned by sizeof(NativeT).
   */
  // TODO(refack): refactor into a non-owning `AliasedBufferBaseView`
  AliasedBufferBase(
      v8::Isolate* isolate,
      const size_t byte_offset,
      const size_t count,
      const AliasedBufferBase<uint8_t, v8::Uint8Array>& backing_buffer,
      const AliasedBufferIndex* index = nullptr)
      : isolate_(isolate),
        count_(count),
        byte_offset_(byte_offset),
        index_(index) {
    if (index != nullptr) {
      // Will be deserialized later.
      return;
    }
    const v8::HandleScope handle_scope(isolate_);
    v8::Local<v8::ArrayBuffer> ab = backing_buffer.GetArrayBuffer();

    // validate that the byte_offset is aligned with sizeof(NativeT)
    CHECK_EQ(byte_offset & (sizeof(NativeT) - 1), 0);
    // validate this fits inside the backing buffer
    CHECK_LE(MultiplyWithOverflowCheck(sizeof(NativeT), count),
             ab->ByteLength() - byte_offset);

    buffer_ = reinterpret_cast<NativeT*>(
        const_cast<uint8_t*>(backing_buffer.GetNativeBuffer() + byte_offset));

    v8::Local<V8T> js_array = V8T::New(ab, byte_offset, count);
    js_array_ = v8::Global<V8T>(isolate, js_array);
  }

  AliasedBufferBase(const AliasedBufferBase& that)
      : isolate_(that.isolate_),
        count_(that.count_),
        byte_offset_(that.byte_offset_),
        buffer_(that.buffer_) {
    DCHECK_NULL(index_);
    js_array_ = v8::Global<V8T>(that.isolate_, that.GetJSArray());
  }

  AliasedBufferBase& operator=(AliasedBufferBase&& that) noexcept {
    DCHECK_NULL(index_);
    this->~AliasedBufferBase();
    isolate_ = that.isolate_;
    count_ = that.count_;
    byte_offset_ = that.byte_offset_;
    buffer_ = that.buffer_;

    js_array_.Reset(isolate_, that.js_array_.Get(isolate_));

    that.buffer_ = nullptr;
    that.js_array_.Reset();
    return *this;
  }

  /**
   * Helper class that is returned from operator[] to support assignment into
   * a specified location.
   */
  class Reference {
   public:
    Reference(AliasedBufferBase<NativeT, V8T>* aliased_buffer, size_t index)
        : aliased_buffer_(aliased_buffer), index_(index) {}

    Reference(const Reference& that)
        : aliased_buffer_(that.aliased_buffer_), index_(that.index_) {}

    inline Reference& operator=(const NativeT& val) {
      aliased_buffer_->SetValue(index_, val);
      return *this;
    }

    inline Reference& operator=(const Reference& val) {
      return *this = static_cast<NativeT>(val);
    }

    operator NativeT() const { return aliased_buffer_->GetValue(index_); }

    inline Reference& operator+=(const NativeT& val) {
      const NativeT current = aliased_buffer_->GetValue(index_);
      aliased_buffer_->SetValue(index_, current + val);
      return *this;
    }

    inline Reference& operator+=(const Reference& val) {
      return this->operator+=(static_cast<NativeT>(val));
    }

    inline Reference& operator-=(const NativeT& val) {
      const NativeT current = aliased_buffer_->GetValue(index_);
      aliased_buffer_->SetValue(index_, current - val);
      return *this;
    }

   private:
    AliasedBufferBase<NativeT, V8T>* aliased_buffer_;
    size_t index_;
  };

  /**
   *  Get the underlying v8 TypedArray overlayed on top of the native buffer
   */
  v8::Local<V8T> GetJSArray() const {
    DCHECK_NULL(index_);
    return js_array_.Get(isolate_);
  }

  void Release() {
    DCHECK_NULL(index_);
    js_array_.Reset();
  }

  /**
   *  Get the underlying v8::ArrayBuffer underlying the TypedArray and
   *  overlaying the native buffer
   */
  v8::Local<v8::ArrayBuffer> GetArrayBuffer() const {
    return GetJSArray()->Buffer();
  }

  /**
   *  Get the underlying native buffer. Note that all reads/writes should occur
   *  through the GetValue/SetValue/operator[] methods
   */
  inline const NativeT* GetNativeBuffer() const {
    DCHECK_NULL(index_);
    return buffer_;
  }

  /**
   *  Synonym for GetBuffer()
   */
  inline const NativeT* operator*() const { return GetNativeBuffer(); }

  /**
   *  Set position index to given value.
   */
  inline void SetValue(const size_t index, NativeT value) {
    DCHECK_LT(index, count_);
    DCHECK_NULL(index_);
    buffer_[index] = value;
  }

  /**
   *  Get value at position index
   */
  inline const NativeT GetValue(const size_t index) const {
    DCHECK_NULL(index_);
    DCHECK_LT(index, count_);
    return buffer_[index];
  }

  /**
   *  Effectively, a synonym for GetValue/SetValue
   */
  Reference operator[](size_t index) {
    DCHECK_NULL(index_);
    return Reference(this, index);
  }

  NativeT operator[](size_t index) const { return GetValue(index); }

  size_t Length() const { return count_; }

  // Should only be used to extend the array.
  // Should only be used on an owning array, not one created as a sub array of
  // an owning `AliasedBufferBase`.
  void reserve(size_t new_capacity) {
    DCHECK_NULL(index_);
    DCHECK_GE(new_capacity, count_);
    DCHECK_EQ(byte_offset_, 0);
    const v8::HandleScope handle_scope(isolate_);

    const size_t old_size_in_bytes = sizeof(NativeT) * count_;
    const size_t new_size_in_bytes =
        MultiplyWithOverflowCheck(sizeof(NativeT), new_capacity);

    // allocate v8 new ArrayBuffer
    v8::Local<v8::ArrayBuffer> ab =
        v8::ArrayBuffer::New(isolate_, new_size_in_bytes);

    // allocate new native buffer
    NativeT* new_buffer = static_cast<NativeT*>(ab->GetBackingStore()->Data());
    // copy old content
    memcpy(new_buffer, buffer_, old_size_in_bytes);

    // allocate v8 TypedArray
    v8::Local<V8T> js_array = V8T::New(ab, byte_offset_, new_capacity);

    // move over old v8 TypedArray
    js_array_ = std::move(v8::Global<V8T>(isolate_, js_array));

    buffer_ = new_buffer;
    count_ = new_capacity;
  }

 private:
  v8::Isolate* isolate_ = nullptr;
  size_t count_ = 0;
  size_t byte_offset_ = 0;
  NativeT* buffer_ = nullptr;
  v8::Global<V8T> js_array_;

  // Deserialize data
  const AliasedBufferIndex* index_ = nullptr;
};

typedef AliasedBufferBase<int32_t, v8::Int32Array> AliasedInt32Array;
typedef AliasedBufferBase<uint8_t, v8::Uint8Array> AliasedUint8Array;
typedef AliasedBufferBase<uint32_t, v8::Uint32Array> AliasedUint32Array;
typedef AliasedBufferBase<double, v8::Float64Array> AliasedFloat64Array;
typedef AliasedBufferBase<uint64_t, v8::BigUint64Array> AliasedBigUint64Array;
}  // namespace pure

#endif  // SRC_ALIASED_BUFFER_H_

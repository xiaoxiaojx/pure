#ifndef SRC_BASE64_H_
#define SRC_BASE64_H_

#include "util.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace pure {

// https://zh-google-styleguide.readthedocs.io/en/latest/google-cpp-styleguide/headers/
// 只有当函数只有 10 行甚至更少时才将其定义为内联函数.
// 定义:
// 当函数被声明为内联函数之后, 编译器会将其内联展开,
// 而不是按通常的函数调用机制进行调用. 优点:
// 只要内联的函数体较小, 内联该函数可以令目标代码更加高效.
// 对于存取函数以及其它函数体比较短, 性能关键的函数, 鼓励使用内联. 缺点:
// 滥用内联将导致程序变得更慢. 内联可能使目标代码量或增或减,
// 这取决于内联函数的大小. 内联非常短小的存取函数通常会减少代码大小,
// 但内联一个相当大的函数将戏剧性的增加代码大小.
// 现代处理器由于更好的利用了指令缓存, 小巧的代码往往执行更快。 结论:
// 一个较为合理的经验准则是, 不要内联超过 10 行的函数. 谨慎对待析构函数,
// 析构函数往往比其表面看起来要更长, 因为有隐含的成员和基类析构函数被调用!
// 另一个实用的经验准则: 内联那些包含循环或 switch 语句的函数常常是得不偿失
// (除非在大多数情况下, 这些循环或 switch 语句从不被执行).
// 有些函数即使声明为内联的也不一定会被编译器内联, 这点很重要;
// 比如虚函数和递归函数就不会被正常内联. 通常,
// 递归函数不应该声明成内联函数.（YuleFox 注:
// 递归调用堆栈的展开并不像循环那么简单, 比如递归层数在编译时可能是未知的,
// 大多数编译器都不支持内联递归函数).
// 虚函数内联的主要原因则是想把它的函数体放在类定义内, 为了图个方便,
// 抑或是当作文档描述其行为, 比如精短的存取函数. Doesn't check for padding at
// the end.  Can be 1-2 bytes over.
static inline constexpr size_t base64_decoded_size_fast(size_t size) {
  // 1-byte input cannot be decoded
  return size > 1 ? (size / 4) * 3 + (size % 4 + 1) / 2 : 0;
}

template <typename TypeName>
size_t base64_decoded_size(const TypeName* src, size_t size) {
  // 1-byte input cannot be decoded
  if (size < 2) return 0;

  if (src[size - 1] == '=') {
    size--;
    if (src[size - 1] == '=') size--;
  }
  return base64_decoded_size_fast(size);
}
}  // namespace pure

#endif  // SRC_BASE64_H_

#include <iostream>
// https://www.zhihu.com/question/51441745/answer/125874662
class A {
 public:
  int i;
  int j;
  int k;
};

void f(const A& a, int A::*pint) {
  std::cout << a.*pint << std::endl;
}

int main() {
  A a = {1, 2, 3};

  f(a, &A::i);
  f(a, &A::j);
  f(a, &A::k);

  return 0;
}

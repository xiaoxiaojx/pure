#include <stdio.h>
#include <string>
using namespace std;

template <int N>
inline int TEST(const char (&data)[N]) {
  return N - 1;
}

template <int N>
inline int TEST2(const int (&data)[N]) {
  return N - 1;
}

int main() {
  char* ps = "C Language";
  char ps2[] = "C Language";

  // ❌
  //   printf("%i", TEST(ps));

  // ✅
  printf("%i", TEST("ssss"));

  // ✅
  printf("%i", TEST(ps2));

  // ✅
  printf("%i", TEST2({1, 2}));

  return 0;
}
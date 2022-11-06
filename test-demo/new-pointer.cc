#include <stdio.h>
#include <string>
using namespace std;

int main() {
  int p = 5;
  int** p1 = new int*(&p);
  printf("%i", **p1);
  return 0;
}
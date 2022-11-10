#include <iostream>

// https://stackoverflow.com/questions/30767243/what-does-mean-in-c

struct C {
    void f(int n) { std::cout << n << '\n'; }
};
int main()
{
    void (C::*p)(int) = &C::f; // p points at member f of class C
    C c;
    c.f(1);
    (c.*p)(1); // prints 1
    C* cptr = &c;
    (cptr->*p)(2); // prints 2
}
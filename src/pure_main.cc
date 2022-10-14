#include <cstdio>
#include "pure.h"

int main(int argc, char *argv[])
{
    // Disable stdio buffering, it interacts poorly with printf()
    // calls elsewhere in the program (e.g., any logging from V8.)
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    return pure::Start(argc, argv);
}
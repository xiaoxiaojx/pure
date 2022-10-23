//
//
//  ________  ___  ___  ________  _______
// |\   __  \|\  \|\  \|\   __  \|\  ___ \     
// \ \  \|\  \ \  \\\  \ \  \|\  \ \   __/|
//  \ \   ____\ \  \\\  \ \   _  _\ \  \_|/__
//   \ \  \___|\ \  \\\  \ \  \\  \\ \  \_|\ \ 
//    \ \__\    \ \_______\ \__\\ _\\ \_______\
//     \|__|     \|_______|\|__|\|__|\|_______|
//
//
//

/**
 * @file pure_main.cc
 * @author xiaoxiaojx (784487301@qq.com)
 * @brief pure
 * @version 0.1
 * @date 2022-10-15
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <cstdio>
#include "pure.h"

int main(int argc, char* argv[]) {
  // Disable stdio buffering, it interacts poorly with printf()
  // calls elsewhere in the program (e.g., any logging from V8.)
  setvbuf(stdout, nullptr, _IONBF, 0);
  setvbuf(stderr, nullptr, _IONBF, 0);
  return pure::Start(argc, argv);
}
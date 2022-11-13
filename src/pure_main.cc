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

#include "cstdio"
#include "pure.h"

#ifdef __linux__
#include <sys/auxv.h>
#endif  // __linux__

namespace pure {
namespace per_process {
extern bool linux_at_secure;
}  // namespace per_process
}  // namespace pure

int main(int argc, char* argv[]) {
#if defined(__linux__)
  // linux capability 权限控制, 比如是否允许读取环境变量, 见
  // ./pure_credentials.cc
  pure::per_process::linux_at_secure = getauxval(AT_SECURE);
#endif

  // 禁用 stdio 缓冲, 其他库比如 v8 可能有日志输出的情况
  setvbuf(stdout, nullptr, _IONBF, 0);
  setvbuf(stderr, nullptr, _IONBF, 0);
  return pure::Start(argc, argv);
}
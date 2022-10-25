//  ________  ___  ___  ________  _______
// |\   __  \|\  \|\  \|\   __  \|\  ___ \     
// \ \  \|\  \ \  \\\  \ \  \|\  \ \   __/|
//  \ \   ____\ \  \\\  \ \   _  _\ \  \_|/__
//   \ \  \___|\ \  \\\  \ \  \\  \\ \  \_|\ \ 
//    \ \__\    \ \_______\ \__\\ _\\ \_______\
//     \|__|     \|_______|\|__|\|__|\|_______|

/**
 * @file pure.cc
 * @author xiaoxiaojx (784487301@qq.com)
 * @brief pure
 * @version 0.1
 * @date 2022-10-15
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <iostream>

#include "pure.h"
#include "uv.h"
#include "v8.h"

#include "util.h"

#include "libplatform/libplatform.h"

#include "pure_binding.h"
#include "pure_main_instance.h"

namespace pure {
using v8::EscapableHandleScope;
using v8::Function;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::String;
using v8::Undefined;
using v8::V8;
using v8::Value;

namespace per_process {
// util.h
// Tells whether the per-process V8::Initialize() is called and
// if it is safe to call v8::Isolate::TryGetCurrent().
bool v8_initialized = false;

std::unique_ptr<v8::Platform> v8_platform;

// 记录下 pure 启动的时间
uint64_t pure_start_time;
}  // namespace per_process

static std::atomic_bool init_called{false};

// Safe to call more than once and from signal handlers.
void ResetStdio() {
  // uv_tty_reset_mode
  // http://docs.libuv.org/en/v1.x/tty.html?highlight=uv_tty_reset_mode#c.uv_tty_reset_mode
  // tcgetattr函数与tcsetattr函数控制终端
  // https://www.cnblogs.com/zhouhbing/p/4129280.html 如果在某处通过
  // uv_tty_set_mode 修改了终端参数, 此处用于复原
  uv_tty_reset_mode();

#ifdef __POSIX__
  for (auto& s : stdio) {
    const int fd = &s - stdio;

    struct stat tmp;
    if (-1 == fstat(fd, &tmp)) {
      CHECK_EQ(errno, EBADF);  // Program closed file descriptor.
      continue;
    }
    // fd 0,1,2 与分别与 stdio s 进行比较, 查看是否已经被修改
    bool is_same_file =
        (s.stat.st_dev == tmp.st_dev && s.stat.st_ino == tmp.st_ino);
    if (!is_same_file) continue;  // Program reopened file descriptor.

    // 如果没有被修改, 则继续运行后面的逻辑
    int flags;
    do flags = fcntl(fd, F_GETFL);
    while (flags == -1 && errno == EINTR);  // NOLINT
    CHECK_NE(flags, -1);

    // 重新设置为 O_NONBLOCK 如果被修改了
    if (O_NONBLOCK & (flags ^ s.flags)) {
      flags &= ~O_NONBLOCK;
      flags |= s.flags & O_NONBLOCK;

      int err;
      do err = fcntl(fd, F_SETFL, flags);
      while (err == -1 && errno == EINTR);  // NOLINT
      CHECK_NE(err, -1);
    }

    // isatty - 测试文件描述符是否指向终端
    if (s.isatty) {
      sigset_t sa;
      int err;

      // We might be a background job that doesn't own the TTY so block SIGTTOU
      // before making the tcsetattr() call, otherwise that signal suspends us.
      sigemptyset(&sa);
      sigaddset(&sa, SIGTTOU);

      CHECK_EQ(0, pthread_sigmask(SIG_BLOCK, &sa, nullptr));
      do err = tcsetattr(fd, TCSANOW, &s.termios);
      while (err == -1 && errno == EINTR);  // NOLINT
      CHECK_EQ(0, pthread_sigmask(SIG_UNBLOCK, &sa, nullptr));

      // Normally we expect err == 0. But if macOS App Sandbox is enabled,
      // tcsetattr will fail with err == -1 and errno == EPERM.
      CHECK_IMPLIES(err != 0, err == -1 && errno == EPERM);
    }
  }
#endif  // __POSIX__
}

int InitializePureWithArgs(std::vector<std::string>* argv,
                           std::vector<std::string>* exec_argv,
                           std::vector<std::string>* errors,
                           ProcessFlags::Flags flags) {
  // Make sure InitializePureWithArgs() is called only once.
  CHECK(!init_called.exchange(true));

  // Initialize pure_start_time to get relative uptime.
  per_process::pure_start_time = uv_hrtime();

  // Register built-in modules
  binding::RegisterBuiltinModules();

  // Make inherited handles noninheritable.
  if (!(flags & ProcessFlags::kEnableStdioInheritance))
    uv_disable_stdio_inheritance();

  // Cache the original command line to be
  // used in diagnostic reports.
  per_process::cli_options->cmdline = *argv;

  HandleEnvOptions(per_process::cli_options->per_isolate->per_env);

  if (!(flags & ProcessFlags::kDisableNodeOptionsEnv)) {
    std::string pure_options;

    if (credentials::SafeGetenv("PURE_OPTIONS", &pure_options)) {
      std::vector<std::string> env_argv =
          ParseNodeOptionsEnvVar(pure_options, errors);

      if (!errors->empty()) return 9;

      // [0] is expected to be the program name, fill it in from the real argv.
      env_argv.insert(env_argv.begin(), argv->at(0));
    }
  }
  // Set the process.title immediately after processing argv if --title is set.
  if (!per_process::cli_options->title.empty())
    uv_set_process_title(per_process::cli_options->title.c_str());

  // We should set pure_is_initialized here instead of in pure::Start,
  // otherwise embedders using pure::Init to initialize everything will not be
  // able to set it and native modules will not load for them.
  pure_is_initialized = true;
  return 0;
}

InitializationResult InitializeOncePerProcess(
    int argc,
    char** argv,
    InitializationSettingsFlags flags,
    ProcessFlags::Flags process_flags) {
  InitializationResult result;

  // atexit 类似于 Node.js process.on("exit", fn)
  atexit(ResetStdio);

  CHECK_GT(argc, 0);

  // uv_setup_args https://docs.libuv.org/en/v1.x/misc.html#c.uv_setup_args
  // 存储程序参数。获取/设置进程标题或可执行路径所必需的。 Libuv 可能会取得 argv
  // 指向的内存的所有权。此函数应在程序启动时仅调用一次。
  argv = uv_setup_args(argc, argv);

  result.args = std::vector<std::string>(argv, argv + argc);
  std::vector<std::string> errors;

  // This needs to run *before* V8::Initialize().
  {
    result.exit_code = InitializePureWithArgs(
        &(result.args), &(result.exec_args), &errors, process_flags);
    for (const std::string& error : errors)
      fprintf(stderr, "%s: %s\n", result.args.at(0).c_str(), error.c_str());
    if (result.exit_code != 0) {
      result.early_return = true;
      return result;
    }
  }

  per_process::v8_platform = v8::platform::NewDefaultPlatform();

  V8::InitializePlatform(per_process::v8_platform.get());

  V8::Initialize();
  return result;
}

InitializationResult InitializeOncePerProcess(int argc, char** argv) {
  return InitializeOncePerProcess(
      argc, argv, kDefaultInitialization, ProcessFlags::Flags::kNoFlags);
}

void TearDownOncePerProcess() {
  per_process::v8_initialized = false;
  V8::Dispose();
  per_process::v8_platform.reset();
}

int Start(int argc, char** argv) {
  InitializationResult result = InitializeOncePerProcess(argc, argv);
  if (result.early_return) {
    return result.exit_code;
  }

  {
    uv_loop_configure(uv_default_loop(), UV_METRICS_IDLE_TIME);

    PureMainInstance main_instance(
        uv_default_loop(), result.args, result.exec_args);
    result.exit_code = main_instance.Run();
  }

  TearDownOncePerProcess();

  return result.exit_code;
}

int Stop(Environment* env) {
  env->ExitEnv();
  return 0;
}
}  // namespace pure

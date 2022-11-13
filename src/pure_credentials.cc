#include "env-inl.h"
#include "pure.h"
#include "util-inl.h"

#if !defined(_MSC_VER)
#include <unistd.h>  // setuid, getuid
#endif
#ifdef __linux__
#include <linux/capability.h>
#include <sys/syscall.h>
#endif  // __linux__

namespace pure {

using v8::Array;
using v8::Context;
using v8::FunctionCallbackInfo;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::Object;
using v8::String;
using v8::TryCatch;
using v8::Uint32;
using v8::Value;

namespace per_process {
bool linux_at_secure = false;
}  // namespace per_process

namespace credentials {

#if defined(__linux__)
// capability https://www.cnblogs.com/sparkdev/p/11417781.html
// 从内核 2.2 开始，Linux 将传统上与超级用户 root
// 关联的特权划分为不同的单元，称为 capabilites。Capabilites 作为线程(Linux
// 并不真正区分进程和线程)的属性存在，每个单元可以独立启用和禁用。如此一来，权限检查的过程就变成了：在执行特权操作时，如果进程的有效身份不是
// root，就去检查是否具有该特权操作所对应的
// capabilites，并以此决定是否可以进行该特权操作。比如要向进程发送信号(kill())，就得具有
// capability CAP_KILL；如果设置系统时间，就得具有 capability CAP_SYS_TIME。
// Returns true if the current process only has the passed-in capability.
bool HasOnly(int capability) {
  DCHECK(cap_valid(capability));

  struct __user_cap_data_struct cap_data[2];
  struct __user_cap_header_struct cap_header_data = {
      _LINUX_CAPABILITY_VERSION_3, getpid()};

  if (syscall(SYS_capget, &cap_header_data, &cap_data) != 0) {
    return false;
  }
  if (capability < 32) {
    return cap_data[0].permitted ==
           static_cast<unsigned int>(CAP_TO_MASK(capability));
  }
  return cap_data[1].permitted ==
         static_cast<unsigned int>(CAP_TO_MASK(capability));
}
#endif

// Look up the environment variable and allow the lookup if the current
// process only has the capability CAP_NET_BIND_SERVICE set. If the current
// process does not have any capabilities set and the process is running as
// setuid root then lookup will not be allowed.
bool SafeGetenv(const char* key, std::string* text, Environment* env) {
#if !defined(__CloudABI__) && !defined(_WIN32)
#if defined(__linux__)
  if ((!HasOnly(CAP_NET_BIND_SERVICE) && per_process::linux_at_secure) ||
      getuid() != geteuid() || getgid() != getegid())
#else
  // geteuid()：返回有效用户的ID。
  // getuid（）：返回实际用户的ID。
  // 有效用户ID（EUID）是你最初执行程序时所用的ID
  //   表示该ID是程序的所有者
  //   真实用户ID（UID）是程序执行过程中采用的ID
  //   该ID表明当前运行位置程序的执行者
  //   举个例子
  //   程序myprogram的所有者为501/anna
  //   以501运行该程序此时UID和EUID都是501
  //   但是由于中间要访问某些系统资源
  //   需要使用root身份
  //   此时UID为0而EUID仍是501
  if (per_process::linux_at_secure || getuid() != geteuid() ||
      getgid() != getegid())
#endif
    goto fail;
#endif

  if (env != nullptr) {
    HandleScope handle_scope(env->isolate());
    TryCatch ignore_errors(env->isolate());
    MaybeLocal<String> maybe_value = env->env_vars()->Get(
        env->isolate(),
        String::NewFromUtf8(env->isolate(), key).ToLocalChecked());
    Local<String> value;
    if (!maybe_value.ToLocal(&value)) goto fail;
    String::Utf8Value utf8_value(env->isolate(), value);
    if (*utf8_value == nullptr) goto fail;
    *text = std::string(*utf8_value, utf8_value.length());
    return true;
  }

  {
    Mutex::ScopedLock lock(per_process::env_var_mutex);

    size_t init_sz = 256;
    MaybeStackBuffer<char, 256> val;
    int ret = uv_os_getenv(key, *val, &init_sz);

    if (ret == UV_ENOBUFS) {
      // Buffer is not large enough, reallocate to the updated init_sz
      // and fetch env value again.
      val.AllocateSufficientStorage(init_sz);
      ret = uv_os_getenv(key, *val, &init_sz);
    }

    if (ret >= 0) {  // Env key value fetch success.
      *text = *val;
      return true;
    }
  }

fail:
  text->clear();
  return false;
}

// TODO
// static void SafeGetenv(const FunctionCallbackInfo<Value>& args) {
//   CHECK(args[0]->IsString());
//   Environment* env = Environment::GetCurrent(args);
//   Isolate* isolate = env->isolate();
//   Utf8Value strenvtag(isolate, args[0]);
//   std::string text;
//   if (!SafeGetenv(*strenvtag, &text, env)) return;
//   Local<Value> result =
//       ToV8Value(isolate->GetCurrentContext(), text).ToLocalChecked();
//   args.GetReturnValue().Set(result);
// }

}  // namespace credentials
}  // namespace pure

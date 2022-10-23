#ifndef SRC_PURE_OPTIONS_INL_H_
#define SRC_PURE_OPTIONS_INL_H_

#include "pure_options.h"

namespace pure {
void PerIsolateOptions::CheckOptions(std::vector<std::string>* errors) {
  per_env->CheckOptions(errors);
}

PerIsolateOptions* PerProcessOptions::get_per_isolate_options() {
  return per_isolate.get();
}

EnvironmentOptions* PerIsolateOptions::get_per_env_options() {
  return per_env.get();
}

void PerProcessOptions::CheckOptions(std::vector<std::string>* errors) {
  // TODO
}
}  // namespace pure

#endif  // SRC_PURE_OPTIONS_INL_H_

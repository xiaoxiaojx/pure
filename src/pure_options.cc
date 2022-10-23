#include "pure_options.h"
#include <cstdlib>
#include "pure_mutex.h"
#include "pure_options-inl.h"

namespace pure {

namespace per_process {
Mutex cli_options_mutex;
std::shared_ptr<PerProcessOptions> cli_options{new PerProcessOptions()};
}  // namespace per_process

void EnvironmentOptions::CheckOptions(std::vector<std::string>* errors) {
  // TODO
}

}  // namespace pure
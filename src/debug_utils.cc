#include "debug_utils-inl.h"

namespace pure {
void FWrite(FILE* file, const std::string& str) {
  auto simple_fwrite = [&]() {
    // The return value is ignored because there's no good way to handle it.
    fwrite(str.data(), str.size(), 1, file);
  };

  if (file != stderr && file != stdout) {
    simple_fwrite();
    return;
  }
  simple_fwrite();
}

}  // namespace pure
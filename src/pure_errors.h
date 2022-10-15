#ifndef SRC_PURE_ERRORS_H_
#define SRC_PURE_ERRORS_H_

#include "v8.h"

namespace pure
{
    namespace errors
    {
        void PerIsolateMessageListener(v8::Local<v8::Message> message,
                                       v8::Local<v8::Value> error);
    }
}

#endif // SRC_PURE_ERRORS_H_

#include "v8.h"
#include "uv.h"
#include "../base_object.h"

namespace pure
{
    void SetIsolateCreateParamsForNode(v8::Isolate::CreateParams *params)
    {
        const uint64_t constrained_memory = uv_get_constrained_memory();
        const uint64_t total_memory = constrained_memory > 0 ? std::min(uv_get_total_memory(), constrained_memory) : uv_get_total_memory();
        if (total_memory > 0)
        {
            // V8 defaults to 700MB or 1.4GB on 32 and 64 bit platforms respectively.
            // This default is based on browser use-cases. Tell V8 to configure the
            // heap based on the actual physical memory.
            params->constraints.ConfigureDefaults(total_memory, 0);
        }
        params->embedder_wrapper_object_index = BaseObject::InternalFields::kSlot;
        params->embedder_wrapper_type_index = std::numeric_limits<int>::max();
    }
}
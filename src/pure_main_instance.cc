#include <assert.h>

#include "v8.h"
#include "uv.h"
#include "pure_main_instance.h"

#include "env.h"
#include "util.h"
#include "api/environment.cc"

using v8::Isolate;

namespace pure
{
    PureMainInstance::PureMainInstance(
        uv_loop_t *event_loop,
        MultiIsolatePlatform *platform,
        const std::vector<std::string> &args,
        const std::vector<std::string> &exec_args)
        : args_(args),
          exec_args_(exec_args),
          array_buffer_allocator_(ArrayBufferAllocator::Create()),
          isolate_(nullptr),
          platform_(platform),
          isolate_data_(),
          isolate_params_(std::make_unique<Isolate::CreateParams>())
          {
              isolate_params_->array_buffer_allocator = array_buffer_allocator_.get();

              isolate_ = Isolate::Allocate();
              assert(isolate_);
              // Register the isolate on the platform before the isolate gets initialized,
              // so that the isolate can access the platform during initialization.
              platform->RegisterIsolate(isolate_, event_loop);
              SetIsolateCreateParamsForNode(isolate_params_.get());
              Isolate::Initialize(isolate_, *isolate_params_);

              // If the indexes are not nullptr, we are not deserializing
            //   isolate_data_ = std::make_unique<IsolateData>(
            //       isolate_,
            //       event_loop,
            //       platform,
            //       array_buffer_allocator_.get(),
            //       snapshot_data == nullptr ? nullptr
            //                                : &(snapshot_data->isolate_data_indices));
            //   IsolateSettings s;
            //   SetIsolateMiscHandlers(isolate_, s);
            //   if (snapshot_data == nullptr)
            //   {
            //       // If in deserialize mode, delay until after the deserialization is
            //       // complete.
            //       SetIsolateErrorHandlers(isolate_, s);
            //   }
            //   isolate_data_->max_young_gen_size =
            //       isolate_params_->constraints.max_young_generation_size_in_bytes();
          };
}

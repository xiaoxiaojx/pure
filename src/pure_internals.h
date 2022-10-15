#ifndef SRC_PURE_INTERNALS_H_
#define SRC_PURE_INTERNALS_H_

#include "v8.h"
#include "pure.h"

namespace pure
{
    class PureArrayBufferAllocator : public ArrayBufferAllocator
    {
    public:
        inline uint32_t *zero_fill_field() { return &zero_fill_field_; }

        void *Allocate(size_t size) override; // Defined in src/node.cc
        void *AllocateUninitialized(size_t size) override;
        void Free(void *data, size_t size) override;
        void *Reallocate(void *data, size_t old_size, size_t size) override;
        virtual void RegisterPointer(void *data, size_t size)
        {
            total_mem_usage_.fetch_add(size, std::memory_order_relaxed);
        }
        virtual void UnregisterPointer(void *data, size_t size)
        {
            total_mem_usage_.fetch_sub(size, std::memory_order_relaxed);
        }

        // PureArrayBufferAllocator *GetImpl() final { return this; }
        inline uint64_t total_mem_usage() const
        {
            return total_mem_usage_.load(std::memory_order_relaxed);
        }

    private:
        uint32_t zero_fill_field_ = 1; // Boolean but exposed as uint32 to JS land.
        std::atomic<size_t> total_mem_usage_{0};
    };
}

#endif  // SRC_PURE_INTERNALS_H_

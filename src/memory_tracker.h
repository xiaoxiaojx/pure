#pragma once

#include <uv.h>
#include <v8.h>
#include "v8-profiler.h"

#include <limits>
#include <queue>
#include <stack>
#include <string>
#include <string>
#include <unordered_map>
#include "util.h"
#include "aliased_buffer.h"

namespace pure
{
    class Environment;
    template <typename T, bool kIsWeak>
    class BaseObjectPtrImpl;
    class MemoryRetainerNode;
    class MemoryTracker;

    class MemoryRetainer
    {
    public:
        virtual ~MemoryRetainer() = default;

        // Subclasses should implement these methods to provide information
        // for the V8 heap snapshot generator.
        // The MemoryInfo() method is assumed to be called within a context
        // where all the edges start from the node of the current retainer,
        // and point to the nodes as specified by tracker->Track* calls.
        virtual void MemoryInfo(MemoryTracker *tracker) const = 0;
        virtual std::string MemoryInfoName() const = 0;
        virtual size_t SelfSize() const = 0;

        virtual v8::Local<v8::Object> WrappedObject() const
        {
            return v8::Local<v8::Object>();
        }

        virtual bool IsRootNode() const { return false; }
    };

    class MemoryTracker
    {
    public:
        // Used to specify node name and size explicitly
        inline void TrackFieldWithSize(const char *edge_name,
                                       size_t size,
                                       const char *node_name = nullptr);
        inline void TrackInlineFieldWithSize(const char *edge_name,
                                             size_t size,
                                             const char *node_name = nullptr);

        // Shortcut to extract the underlying object out of the smart pointer
        template <typename T, typename D>
        inline void TrackField(const char *edge_name,
                               const std::unique_ptr<T, D> &value,
                               const char *node_name = nullptr);

        template <typename T>
        inline void TrackField(const char *edge_name,
                               const std::shared_ptr<T> &value,
                               const char *node_name = nullptr);

        template <typename T, bool kIsWeak>
        void TrackField(const char *edge_name,
                        const BaseObjectPtrImpl<T, kIsWeak> &value,
                        const char *node_name = nullptr);

        // For containers, the elements will be graphed as grandchildren nodes
        // if the container is not empty.
        // By default, we assume the parent count the stack size of the container
        // into its SelfSize so that will be subtracted from the parent size when we
        // spin off a new node for the container.
        // TODO(joyeecheung): use RTTI to retrieve the class name at runtime?
        template <typename T, typename Iterator = typename T::const_iterator>
        inline void TrackField(const char *edge_name,
                               const T &value,
                               const char *node_name = nullptr,
                               const char *element_name = nullptr,
                               bool subtract_from_self = true);
        template <typename T>
        inline void TrackField(const char *edge_name,
                               const std::queue<T> &value,
                               const char *node_name = nullptr,
                               const char *element_name = nullptr);
        template <typename T, typename U>
        inline void TrackField(const char *edge_name,
                               const std::pair<T, U> &value,
                               const char *node_name = nullptr);

        // For the following types, node_name will be ignored and predefined names
        // will be used instead. They are only in the signature for template
        // expansion.
        inline void TrackField(const char *edge_name,
                               const MemoryRetainer &value,
                               const char *node_name = nullptr);
        inline void TrackField(const char *edge_name,
                               const MemoryRetainer *value,
                               const char *node_name = nullptr);
        template <typename T>
        inline void TrackField(const char *edge_name,
                               const std::basic_string<T> &value,
                               const char *node_name = nullptr);
        template <typename T,
                  typename test_for_number = typename std::
                      enable_if<std::numeric_limits<T>::is_specialized, bool>::type,
                  typename dummy = bool>
        inline void TrackField(const char *edge_name,
                               const T &value,
                               const char *node_name = nullptr);
        template <typename T>
        void TrackField(const char *edge_name,
                        const v8::Eternal<T> &value,
                        const char *node_name);
        template <typename T>
        inline void TrackField(const char *edge_name,
                               const v8::PersistentBase<T> &value,
                               const char *node_name = nullptr);
        template <typename T>
        inline void TrackField(const char *edge_name,
                               const v8::Local<T> &value,
                               const char *node_name = nullptr);
        template <typename T>
        inline void TrackField(const char *edge_name,
                               const MallocedBuffer<T> &value,
                               const char *node_name = nullptr);
        inline void TrackField(const char *edge_name,
                               const v8::BackingStore *value,
                               const char *node_name = nullptr);
        inline void TrackField(const char *edge_name,
                               const uv_buf_t &value,
                               const char *node_name = nullptr);
        inline void TrackField(const char *edge_name,
                               const uv_timer_t &value,
                               const char *node_name = nullptr);
        inline void TrackField(const char *edge_name,
                               const uv_async_t &value,
                               const char *node_name = nullptr);
        inline void TrackInlineField(const char *edge_name,
                                     const uv_async_t &value,
                                     const char *node_name = nullptr);
        template <class NativeT, class V8T>
        inline void TrackField(const char *edge_name,
                               const AliasedBufferBase<NativeT, V8T> &value,
                               const char *node_name = nullptr);

        // Put a memory container into the graph, create an edge from
        // the current node if there is one on the stack.
        inline void Track(const MemoryRetainer *retainer,
                          const char *edge_name = nullptr);

        // Useful for parents that do not wish to perform manual
        // adjustments to its `SelfSize()` when embedding retainer
        // objects inline.
        // Put a memory container into the graph, create an edge from
        // the current node if there is one on the stack - there should
        // be one, of the container object which the current field is part of.
        // Reduce the size of memory from the container so as to avoid
        // duplication in accounting.
        inline void TrackInlineField(const MemoryRetainer *retainer,
                                     const char *edge_name = nullptr);

        inline v8::EmbedderGraph *graph() { return graph_; }
        inline v8::Isolate *isolate() { return isolate_; }

        inline explicit MemoryTracker(v8::Isolate *isolate,
                                      v8::EmbedderGraph *graph)
            : isolate_(isolate), graph_(graph) {}

    private:
        typedef std::unordered_map<const MemoryRetainer *, MemoryRetainerNode *>
            NodeMap;

        inline MemoryRetainerNode *CurrentNode() const;
        inline MemoryRetainerNode *AddNode(const MemoryRetainer *retainer,
                                           const char *edge_name = nullptr);
        inline MemoryRetainerNode *PushNode(const MemoryRetainer *retainer,
                                            const char *edge_name = nullptr);
        inline MemoryRetainerNode *AddNode(const char *node_name,
                                           size_t size,
                                           const char *edge_name = nullptr);
        inline MemoryRetainerNode *PushNode(const char *node_name,
                                            size_t size,
                                            const char *edge_name = nullptr);
        inline void PopNode();

        v8::Isolate *isolate_;
        v8::EmbedderGraph *graph_;
        std::stack<MemoryRetainerNode *> node_stack_;
        NodeMap seen_;
    };
}
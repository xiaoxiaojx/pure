#ifndef SRC_ENV_INL_H_
#define SRC_ENV_INL_H_

#include "env.h"
#include "util.h"
#include "v8.h"

namespace pure
{
    // https://zhuanlan.zhihu.com/p/152055532
    // 内联函数语法: inline要起作用,必须要与函数定义放在一起，而不是函数的声明
    // 内联函数的作用: 当编译器处理调用内联函数的语句时，不会将该语句编译成函数调用的指令，而是直接将整个函数体的代码插人调用语句处，就像整个函数体在调用处被重写了一遍一样，在执行时是顺序执行，而不会进行跳

    inline Environment *Environment::GetCurrent(v8::Isolate *isolate)
    {
        if (UNLIKELY(!isolate->InContext()))
            return nullptr;
        v8::HandleScope handle_scope(isolate);
        return GetCurrent(isolate->GetCurrentContext());
    }

    inline Environment *Environment::GetCurrent(v8::Local<v8::Context> context)
    {
        if (UNLIKELY(context.IsEmpty()))
        {
            return nullptr;
        }
        if (UNLIKELY(context->GetNumberOfEmbedderDataFields() <=
                     ContextEmbedderIndex::kContextTag))
        {
            return nullptr;
        }
        if (UNLIKELY(context->GetAlignedPointerFromEmbedderData(
                         ContextEmbedderIndex::kContextTag) !=
                     Environment::kNodeContextTagPtr))
        {
            return nullptr;
        }
        return static_cast<Environment *>(
            context->GetAlignedPointerFromEmbedderData(
                ContextEmbedderIndex::kEnvironment));
    }

    inline Environment *Environment::GetCurrent(
        const v8::FunctionCallbackInfo<v8::Value> &info)
    {
        return GetCurrent(info.GetIsolate()->GetCurrentContext());
    }

    template <typename T>
    inline Environment *Environment::GetCurrent(
        const v8::PropertyCallbackInfo<T> &info)
    {
        return GetCurrent(info.GetIsolate()->GetCurrentContext());
    }

    v8::Local<v8::Context> Environment::context() const
    {
        return PersistentToLocal::Strong(context_);
    }

    inline v8::Isolate *IsolateData::isolate() const
    {
        return isolate_;
    }

    inline uv_loop_t *IsolateData::event_loop() const
    {
        return event_loop_;
    }

    inline void Environment::AssignToContext(v8::Local<v8::Context> context,
                                             const ContextInfo &info)
    {
        context->SetAlignedPointerInEmbedderData(
            ContextEmbedderIndex::kEnvironment, this);
        // Used by Environment::GetCurrent to know that we are on a node context.
        context->SetAlignedPointerInEmbedderData(
            ContextEmbedderIndex::kContextTag, Environment::kNodeContextTagPtr);
        // Used to retrieve bindings
        // context->SetAlignedPointerInEmbedderData(
        //     ContextEmbedderIndex::kBindingListIndex, &(this->bindings_));

// #if HAVE_INSPECTOR
//         inspector_agent()->ContextCreated(context, info);
// #endif // HAVE_INSPECTOR

//         this->async_hooks()->AddContext(context);
    }
}

#endif // SRC_ENV_INL_H_

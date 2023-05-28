#pragma once

#include "InputCommon.h"
#include <unordered_set>
#include <functional>

namespace Xunlan
{
    class InputHandlerBase
    {
    protected:

        InputHandlerBase();
        virtual ~InputHandlerBase();

    public:

        virtual void OnEvent(InputDevice device, InputCode code, const InputValue& value) = 0;
    };

    template<typename T>
    class InputHandler final : public InputHandlerBase
    {
    public:

        using CallbackFunc = void(T::*)(InputDevice, InputCode, const InputValue&);

        void Register(T* obj, CallbackFunc func) { m_callbacks.insert({ obj, func }); }
        void Unregister(T* obj, CallbackFunc func) { m_callbacks.erase({ obj, func }); }

        virtual void OnEvent(InputDevice device, InputCode code, const InputValue& value) override
        {
            assert(device < InputDevice::COUNT);

            for (const CallbackInfo& info : m_callbacks)
            {
                (info.m_obj->*info.m_func)(device, code, value);
            }
        }

    private:

        struct CallbackInfo final
        {
            T* m_obj = nullptr;
            CallbackFunc m_func;

            friend bool operator==(const CallbackInfo& lhs, const CallbackInfo& rhs)
            {
                return lhs.m_obj == rhs.m_obj && lhs.m_func == rhs.m_func;
            }

            struct Hash final
            {
                size_t operator()(const CallbackInfo& key) const { return (size_t)key.m_obj; }
            };
        };

        std::unordered_set<CallbackInfo, typename CallbackInfo::Hash> m_callbacks;
    };
}
#pragma once

#include "InputCommon.h"
#include "InputHandler.h"
#include <unordered_set>
#include <unordered_map>

namespace Xunlan
{
    class InputSystem final : public Singleton<InputSystem>
    {
        friend class Singleton<InputSystem>;

    private:

        InputSystem() = default;

    public:

        void RegisterHandler(InputHandlerBase* handler) { m_handlers.insert(handler); }
        void UnregisterHandler(InputHandlerBase* handler) { m_handlers.erase(handler); }

        InputValue Get(InputDevice device, InputCode code);
        void Set(InputDevice device, InputCode code, const Math::float3& value);

    private:

        using InputKey = uint64;

        InputKey GetKey(InputDevice device, InputCode code) { return ((uint64)device << 32) | (uint64)code; }

    private:

        std::unordered_map<InputKey, InputValue> m_inputValues;
        std::unordered_set<InputHandlerBase*> m_handlers;
    };
}
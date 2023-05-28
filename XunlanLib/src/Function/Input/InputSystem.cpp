#include "InputSystem.h"

namespace Xunlan
{
    InputValue InputSystem::Get(InputDevice device, InputCode code)
    {
        assert(device < InputDevice::COUNT);
        const uint64 key = GetKey(device, code);
        return m_inputValues[key];
    }
    void InputSystem::Set(InputDevice device, InputCode code, const Math::float3& value)
    {
        assert(device < InputDevice::COUNT);
        const uint64 key = GetKey(device, code);

        InputValue& inputValue = m_inputValues[key];
        inputValue.m_prev = inputValue.m_curr;
        inputValue.m_curr = value;

        for (InputHandlerBase* handler : m_handlers)
        {
            handler->OnEvent(device, code, inputValue);
        }
    }
}
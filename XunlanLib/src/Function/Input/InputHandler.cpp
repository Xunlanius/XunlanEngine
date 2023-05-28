#include "InputHandler.h"
#include "InputSystem.h"

namespace Xunlan
{
    InputHandlerBase::InputHandlerBase()
    {
        Singleton<InputSystem>::Instance().RegisterHandler(this);
    }
    InputHandlerBase::~InputHandlerBase()
    {
        Singleton<InputSystem>::Instance().UnregisterHandler(this);
    }
}
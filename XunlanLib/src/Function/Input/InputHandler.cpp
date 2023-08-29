#include "InputHandler.h"
#include "InputSystem.h"

namespace Xunlan
{
    InputHandlerBase::InputHandlerBase()
    {
        InputSystem::Instance().RegisterHandler(this);
    }
    InputHandlerBase::~InputHandlerBase()
    {
        InputSystem::Instance().UnregisterHandler(this);
    }
}
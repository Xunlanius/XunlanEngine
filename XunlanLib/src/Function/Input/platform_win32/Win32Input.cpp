#ifdef _WIN64

#include "Win32Input.h"
#include "src/Function/Input/InputSystem.h"

namespace Xunlan
{
    namespace
    {
        constexpr InputCode g_vkMapping[256] = {
            /* 0x00 */ InputCode::INVALID,
            /* 0x01 */ InputCode::MOUSE_LEFT,
            /* 0x02 */ InputCode::MOUSE_RIGHT,
            /* 0x03 */ InputCode::INVALID,
            /* 0x04 */ InputCode::MOUSE_MID,
            /* 0x05 */ InputCode::INVALID,
            /* 0x06 */ InputCode::INVALID,
            /* 0x07 */ InputCode::INVALID,
            /* 0x08 */ InputCode::KEY_BACKSPACE,
            /* 0x09 */ InputCode::KEY_TAB,
            /* 0x0A */ InputCode::INVALID,
            /* 0x0B */ InputCode::INVALID,
            /* 0x0C */ InputCode::INVALID,
            /* 0x0D */ InputCode::KEY_RETURN,
            /* 0x0E */ InputCode::INVALID,
            /* 0x0F */ InputCode::INVALID,

            /* 0x10 */ InputCode::KEY_SHIFT,
            /* 0x11 */ InputCode::KEY_CTRL,
            /* 0x12 */ InputCode::KEY_ALT,
            /* 0x13 */ InputCode::KEY_PAUSE,
            /* 0x14 */ InputCode::KEY_CAPS,
            /* 0x15 */ InputCode::INVALID,
            /* 0x16 */ InputCode::INVALID,
            /* 0x17 */ InputCode::INVALID,
            /* 0x18 */ InputCode::INVALID,
            /* 0x19 */ InputCode::INVALID,
            /* 0x1A */ InputCode::INVALID,
            /* 0x1B */ InputCode::KEY_ESCAPE,
            /* 0x1C */ InputCode::INVALID,
            /* 0x1D */ InputCode::INVALID,
            /* 0x1E */ InputCode::INVALID,
            /* 0x1F */ InputCode::INVALID,

            /* 0x20 */ InputCode::KEY_SPACE,
            /* 0x21 */ InputCode::INVALID,
            /* 0x22 */ InputCode::INVALID,
            /* 0x23 */ InputCode::INVALID,
            /* 0x24 */ InputCode::INVALID,
            /* 0x25 */ InputCode::KEY_LEFT,
            /* 0x26 */ InputCode::KEY_UP,
            /* 0x27 */ InputCode::KEY_RIGHT,
            /* 0x28 */ InputCode::KEY_DOWN,
            /* 0x29 */ InputCode::INVALID,
            /* 0x2A */ InputCode::INVALID,
            /* 0x2B */ InputCode::INVALID,
            /* 0x2C */ InputCode::INVALID,
            /* 0x2D */ InputCode::INVALID,
            /* 0x2E */ InputCode::INVALID,
            /* 0x2F */ InputCode::INVALID,

            /* 0x30 */ InputCode::KEY_0,
            /* 0x31 */ InputCode::KEY_1,
            /* 0x32 */ InputCode::KEY_2,
            /* 0x33 */ InputCode::KEY_3,
            /* 0x34 */ InputCode::KEY_4,
            /* 0x35 */ InputCode::KEY_5,
            /* 0x36 */ InputCode::KEY_6,
            /* 0x37 */ InputCode::KEY_7,
            /* 0x38 */ InputCode::KEY_8,
            /* 0x39 */ InputCode::KEY_9,
            /* 0x3A */ InputCode::INVALID,
            /* 0x3B */ InputCode::INVALID,
            /* 0x3C */ InputCode::INVALID,
            /* 0x3D */ InputCode::INVALID,
            /* 0x3E */ InputCode::INVALID,
            /* 0x3F */ InputCode::INVALID,

            /* 0x40 */ InputCode::INVALID,
            /* 0x41 */ InputCode::KEY_A,
            /* 0x42 */ InputCode::KEY_B,
            /* 0x43 */ InputCode::KEY_C,
            /* 0x44 */ InputCode::KEY_D,
            /* 0x45 */ InputCode::KEY_E,
            /* 0x46 */ InputCode::KEY_F,
            /* 0x47 */ InputCode::KEY_G,
            /* 0x48 */ InputCode::KEY_H,
            /* 0x49 */ InputCode::KEY_I,
            /* 0x4A */ InputCode::KEY_J,
            /* 0x4B */ InputCode::KEY_K,
            /* 0x4C */ InputCode::KEY_L,
            /* 0x4D */ InputCode::KEY_M,
            /* 0x4E */ InputCode::KEY_N,
            /* 0x4F */ InputCode::KEY_O,

            /* 0x50 */ InputCode::KEY_P,
            /* 0x51 */ InputCode::KEY_Q,
            /* 0x52 */ InputCode::KEY_R,
            /* 0x53 */ InputCode::KEY_S,
            /* 0x54 */ InputCode::KEY_T,
            /* 0x55 */ InputCode::KEY_U,
            /* 0x56 */ InputCode::KEY_V,
            /* 0x57 */ InputCode::KEY_W,
            /* 0x58 */ InputCode::KEY_X,
            /* 0x59 */ InputCode::KEY_Y,
            /* 0x5A */ InputCode::KEY_Z,
            /* 0x5B */ InputCode::INVALID,
            /* 0x5C */ InputCode::INVALID,
            /* 0x5D */ InputCode::INVALID,
            /* 0x5E */ InputCode::INVALID,
            /* 0x5F */ InputCode::INVALID,
        };

        enum class ModifierFlags : byte
        {
            LEFT_SHIFT = 0x10,
            LEFT_CTRL = 0x20,
            LEFT_ALT = 0x40,

            RIGHT_SHIFT = 0x01,
            RIGHT_CTRL = 0x02,
            RIGHT_ALT = 0x04,
        };

        bool IsKeyPressed(int key) { return ::GetKeyState(key) < 0; }
        Math::float2 GetMousePos(LPARAM lParam) { return { (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam) }; }

        byte g_modifierState = 0;
    }

    HRESULT ProcessInputMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            const InputCode code = g_vkMapping[wParam];
            if (code != InputCode::INVALID)
            {
                InputSystem::Instance().Set(InputDevice::KEYBOARD, code, { 1.0f, 0.0f, 0.0f });
            }
        }
        break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            const InputCode code = g_vkMapping[wParam];
            if (code != InputCode::INVALID)
            {
                InputSystem::Instance().Set(InputDevice::KEYBOARD, code, { 0.0f, 0.0f, 0.0f });
            }
        }
        break;

        case WM_MOUSEMOVE:
        {
            InputSystem& inputSystem = InputSystem::Instance();
            const Math::float2 pos = GetMousePos(lParam);
            inputSystem.Set(InputDevice::MOUSE, InputCode::MOUSE_POS_X, { pos.x, 0.0f, 0.0f });
            inputSystem.Set(InputDevice::MOUSE, InputCode::MOUSE_POS_Y, { pos.y, 0.0f, 0.0f });
            inputSystem.Set(InputDevice::MOUSE, InputCode::MOUSE_POS, { pos.x, pos.y, 0.0f });
        }
        break;

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        {
            ::SetCapture(hwnd);
            const InputCode code = msg == WM_LBUTTONDOWN ? InputCode::MOUSE_LEFT : msg == WM_RBUTTONDOWN ? InputCode::MOUSE_RIGHT : InputCode::MOUSE_MID;
            InputSystem::Instance().Set(InputDevice::MOUSE, code, { 1.0f, 0.0f, 0.0f });
        }
        break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
            ::ReleaseCapture();
            const InputCode code = msg == WM_LBUTTONUP ? InputCode::MOUSE_LEFT : msg == WM_RBUTTONUP ? InputCode::MOUSE_RIGHT : InputCode::MOUSE_MID;
            InputSystem::Instance().Set(InputDevice::MOUSE, code, { 0.0f, 0.0f, 0.0f });
        }
        break;

        case WM_MOUSEWHEEL:
        {
            const float wheelDelta = (float)GET_WHEEL_DELTA_WPARAM(wParam);
            InputSystem::Instance().Set(InputDevice::MOUSE, InputCode::MOUSE_WHEEL, { wheelDelta, 0.0f, 0.0f });
        }
        break;
        }

        return S_OK;
    }
}

#endif
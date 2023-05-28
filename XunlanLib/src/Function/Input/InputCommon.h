#pragma once

#include "src/Common/Common.h"
#include "src/Utility/Math/MathTypes.h"
#include <string>

namespace Xunlan
{
    enum class Axis : uint32
    {
        X = 0,
        Y = 1,
        Z = 2,
    };

    enum class ModifierKey : uint32
    {
        NONE = 0x00,

        LEFT_SHIFT = 0x01,
        RIGHT_SHIFT = 0x02,
        SHIFT = LEFT_SHIFT | RIGHT_SHIFT,

        LEFT_CONTROL = 0x04,
        RIGHT_CONTROL = 0x08,
        CONTROL = LEFT_CONTROL | RIGHT_CONTROL,

        LEFT_ALT = 0x10,
        RIGHT_ALT = 0x20,
        ALT = LEFT_ALT | RIGHT_ALT,
    };

    enum class InputCode : uint32
    {
        INVALID,

        MOUSE_POS,
        MOUSE_POS_X,
        MOUSE_POS_Y,
        MOUSE_LEFT,
        MOUSE_RIGHT,
        MOUSE_MID,
        MOUSE_WHEEL,

        KEY_BACKSPACE,
        KEY_TAB,
        KEY_RETURN,
        KEY_SHIFT,
        KEY_LEFT_SHIFT,
        KEY_RIGHT_SHIFT,
        KEY_CTRL,
        KEY_LEFT_CTRL,
        KEY_RIGHT_CTRL,
        KEY_ALT,
        KEY_LEFT_ALT,
        KEY_RIGHT_ALT,
        KEY_SPACE,
        KEY_CAPS,
        KEY_PAUSE,
        KEY_ESCAPE,
        KEY_LEFT,
        KEY_UP,
        KEY_RIGHT,
        KEY_DOWN,

        KEY_A,
        KEY_B,
        KEY_C,
        KEY_D,
        KEY_E,
        KEY_F,
        KEY_G,
        KEY_H,
        KEY_I,
        KEY_J,
        KEY_K,
        KEY_L,
        KEY_M,
        KEY_N,
        KEY_O,
        KEY_P,
        KEY_Q,
        KEY_R,
        KEY_S,
        KEY_T,
        KEY_U,
        KEY_V,
        KEY_W,
        KEY_X,
        KEY_Y,
        KEY_Z,

        KEY_0,
        KEY_1,
        KEY_2,
        KEY_3,
        KEY_4,
        KEY_5,
        KEY_6,
        KEY_7,
        KEY_8,
        KEY_9,

        KEY_F1,
        KEY_F2,
        KEY_F3,
        KEY_F4,
        KEY_F5,
        KEY_F6,
        KEY_F7,
        KEY_F8,
        KEY_F9,
        KEY_F10,
        KEY_F11,
        KEY_F12,
    };

    struct InputValue final
    {
        Math::float3 m_prev;
        Math::float3 m_curr;
    };

    enum class InputDevice : uint32
    {
        KEYBOARD,
        MOUSE,

        COUNT,
    };

    struct ActionBinding final
    {
        std::string m_action;
        InputDevice m_device = {};
        InputCode m_code = {};

        float m_multiplier = 0.0f;
        bool m_isDiscrete = true;
        Axis m_sourceAxis = {};
        Axis m_axis = {};
        ModifierKey m_modifier = {};
    };
}
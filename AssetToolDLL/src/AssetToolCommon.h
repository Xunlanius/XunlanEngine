#pragma once

#include "Common/Common.h"

#if !defined EDITOR_INTERFACE
    #define EDITOR_INTERFACE extern "C" __declspec(dllexport)
#endif
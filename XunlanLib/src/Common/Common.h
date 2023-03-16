#pragma once

#include <array>
#include <vector>

#include <tuple>
#include <string>

#include <utility>
#include <algorithm>

#include <iostream>
#include <memory>
#include <cstdint>
#include <cassert>

#if defined _WIN64

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <windowsx.h>
#include <wrl.h>
#include <atlsafe.h>

#endif

#include "Common/ID.h"
#include "Common/DataTypes.h"
#include "ECS/ECS.h"
#pragma once

#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <tuple>

#include <iostream>
#include <string>
#include <memory>
#include <cstdint>
#include <utility>
#include <typeinfo>
#include <algorithm>
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
#include "Common/ECS/ECS.h"
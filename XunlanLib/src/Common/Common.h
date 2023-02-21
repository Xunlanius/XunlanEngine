#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <cstdint>
#include <utility>
#include <typeinfo>
#include <algorithm>
#include <functional>
#include <ctime>
#include <chrono>
#include <cassert>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <array>
#include <vector>
#include <list>
#include <queue>
#include <deque>
#include <stack>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <tuple>

#if defined _WIN64

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

    #include <Windows.h>
    #include <windowsx.h>
    #include <wrl.h>
    #include <atlsafe.h>

    #pragma comment(lib, "d3d12.lib")
    #pragma comment(lib, "dxgi.lib")
    #pragma comment(lib, "d3dcompiler.lib")
    #pragma comment(lib, "dxguid.lib")

    #include <d3d12.h>
    #include <dxgi1_6.h>
    #include <d3dcompiler.h>
    #include <DirectXMath.h>
    #include <DirectXColors.h>
    #include <DirectXCollision.h>
#endif

#include "Common/ID.h"
#include "Common/DataTypes.h"
#include "Common/ECS/ECS.h"

#include "Utilities/Math.h"
#include "Utilities/Utilities.h"

#if defined _DEBUG
    #define DEBUG_OPER(x) x
#else
    #define DEBUG_OPER(x) ((void)0)
#endif
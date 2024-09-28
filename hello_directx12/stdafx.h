// Liam Wynn, 9/26/2024, Hello DirectX 12

//
// A simple include file to combine a lot of reused
// includes like DirectX, WIN32API, and Standard C++
// stuff.
//

#pragma once

// Excludes rarely used stuff from Windows headers.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // ! WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <d3dx12.h>

#include <string>
#include <vector>
#include <cassert>

#include <wrl.h>
#include <shellapi.h>

using namespace Microsoft::WRL;
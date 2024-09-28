// Liam Wynn, 9/26/2024, Hello DirectX 12

#pragma once

#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include <exception>

inline void throw_if_failed(const HRESULT result) {
	if (FAILED(result)) {
		throw std::exception();
	}
}


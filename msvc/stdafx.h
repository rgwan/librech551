// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#include "targetver.h"

// Suppress unsafe warnings
//#define _CRT_SECURE_NO_DEPRECATE
//#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <tchar.h>

#ifdef _MSC_VER
// MSVC
#ifdef _WIN64
#pragma comment(lib, "libusb\\x64\\libusb-1.0.lib")
#else
#pragma comment(lib, "libusb\\x86\\libusb-1.0.lib")
#endif

#endif	// _MSC_VER
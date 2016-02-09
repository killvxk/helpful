// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#ifdef UNICODE
#define tstrcpy_s(A, B, C) wcscpy_s(A, B, C)
#else // UNICODE
#define tstrcpy_s(A, B, C) strcpy_s(A, B, C)
#endif // UNICODE

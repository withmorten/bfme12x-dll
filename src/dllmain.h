#pragma once

#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
#include <direct.h>
#include <stdlib.h>

typedef int32_t ssize_t;
#include <util.h>

FLOAT GetPrivateProfileFloatA(LPCSTR lpAppName, LPCTSTR lpKeyName, FLOAT fDefault, LPCSTR lpFileName);
#define GetPrivateProfileFloat GetPrivateProfileFloatA

UINT get_dll_int(LPCTSTR lpKeyName, INT nDefault);
UINT get_dll_bool(LPCTSTR lpKeyName, INT nDefault);
DWORD get_dll_string(LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize);
FLOAT get_dll_float(LPCTSTR lpKeyName, FLOAT fDefault);

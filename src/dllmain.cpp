#include "dllmain.h"
#include "patch.h"

FLOAT GetPrivateProfileFloatA(LPCSTR lpAppName, LPCTSTR lpKeyName, FLOAT fDefault, LPCSTR lpFileName)
{
	CHAR lpReturnedString[1024];

	DWORD dwVal = GetPrivateProfileString(lpAppName, lpKeyName, "", lpReturnedString, sizeof(lpReturnedString), lpFileName);

	return dwVal ? (FLOAT)atof(lpReturnedString) : fDefault;
}

static char ini_path[MAX_PATH];

UINT get_dll_int(LPCTSTR lpKeyName, INT nDefault)
{
	return GetPrivateProfileInt(DLL_NAME, lpKeyName, nDefault, ini_path);
}

UINT get_dll_bool(LPCTSTR lpKeyName, INT nDefault)
{
	return get_dll_int(lpKeyName, nDefault);
}

DWORD get_dll_string(LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize)
{
	return GetPrivateProfileString(DLL_NAME, lpKeyName, lpDefault, lpReturnedString, nSize, ini_path);
}

FLOAT get_dll_float(LPCTSTR lpKeyName, FLOAT fDefault)
{
	return GetPrivateProfileFloat(DLL_NAME, lpKeyName, fDefault, ini_path);
}

void init_private_profile()
{
	_getcwd(ini_path, sizeof(ini_path));
	strscat(ini_path, "\\" DLL_NAME ".ini");
}

void init_console()
{
	AllocConsole();

	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		init_private_profile();
		if (stristr(GetCommandLine(), "-console") || get_dll_bool("AllocConsole", FALSE)) init_console();

		patch();
	}

	return TRUE;
}

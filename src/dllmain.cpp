#include "dllmain.h"
#include "patch.h"

static char ini_path[MAX_PATH];

UINT get_private_profile_int(LPCTSTR lpKeyName, INT nDefault)
{
	return GetPrivateProfileInt(DLL_NAME, lpKeyName, nDefault, ini_path);
}

UINT get_private_profile_bool(LPCTSTR lpKeyName, INT nDefault)
{
	return get_private_profile_int(lpKeyName, nDefault);
}

DWORD get_private_profile_string(LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize)
{
	return GetPrivateProfileString(DLL_NAME, lpKeyName, lpDefault, lpReturnedString, nSize, ini_path);
}

FLOAT get_private_profile_float(LPCTSTR lpKeyName, LPCTSTR lpDefault)
{
	CHAR lpReturnedString[MAX_PATH];

	get_private_profile_string(lpKeyName, lpDefault, lpReturnedString, sizeof(lpReturnedString));

	return (FLOAT)atof(lpReturnedString);
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
		if (get_private_profile_bool("AllocConsole", FALSE)) init_console();

		patch();
	}

	return TRUE;
}

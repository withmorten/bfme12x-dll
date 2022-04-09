#include "patch.h"
#include "bfme1.h"

namespace bfme1
{
int GetGameLogicRandomValue(int min, int max, char *file, int line) { XCALL(0x00401BAE); }

bool g_bUseRandomValue;

struct GlobalData
{
	enum TimeOfDay
	{
		NONE,
		MORNING,
		AFTERNOON,
		EVENING,
		NIGHT,
		INTERPOLATE,
	};

	void setTimeOfDay(TimeOfDay t) { XCALL(0x0040BA64); }

	void _setTimeOfDay_forced(TimeOfDay t)
	{
		UINT u = get_private_profile_int("force_time_of_day", NONE);

		if (u >= MORNING && u <= NIGHT)
		{
			FIELD(TimeOfDay, this, 0x218) = (TimeOfDay)u;
			setTimeOfDay(FIELD(TimeOfDay, this, 0x218));
		}
		else
		{
			setTimeOfDay(t);
		}
	}

	void _setTimeOfDay_random(TimeOfDay t)
	{
		// going by worldbuilder asserts, you shouldn't call this outside of an actual gamelogic phase ... but eh
		TimeOfDay r = (TimeOfDay)GetGameLogicRandomValue(MORNING, NIGHT, __FILE__, __LINE__);

		if (g_bUseRandomValue)
		{
			FIELD(TimeOfDay, this, 0x218) = r;
		}

		setTimeOfDay(FIELD(TimeOfDay, this, 0x218));
	}
};

extern GlobalData *&TheWriteableGlobalData;

extern uint32_t &g_flags;

extern bool &g_bHouseColor;

struct INI
{
	static void parseReal(INI *ini, void *formal, void *store, const void *user_data) { XCALL(0x00C52B20); }
	static void parseInt(INI *ini, void *formal, void *store, const void *user_data) { XCALL(0x00C52A60); }
	static void parseUnsignedShort(INI *ini, void *formal, void *store, const void *user_data) { XCALL(0x00C529E0); }

	static void _parseRealBuildTime(INI *ini, void *formal, void *store, const void *user_data)
	{
		parseReal(ini, formal, store, user_data);

		float val;
		memcpy(&val, store, sizeof(val));
		if (val > 5.0f) val = 5.0f;
		memcpy(store, &val, sizeof(val));
	}

	static void _parseIntBuildCost(INI *ini, void *formal, void *store, const void *user_data)
	{
		parseInt(ini, formal, store, user_data);

		int val;
		memcpy(&val, store, sizeof(val));
		if (val > 10) val = 10;
		memcpy(store, &val, sizeof(val));
	}

	static void _parseUnsignedShortBuildCost(INI *ini, void *formal, void *store, const void *user_data)
	{
		parseUnsignedShort(ini, formal, store, user_data);

		unsigned short val;
		memcpy(&val, store, sizeof(val));
		if (val > 10) val = 10;
		memcpy(store, &val, sizeof(val));
	}

	static void _parseRealRebuildTimeSeconds(INI *ini, void *formal, void *store, const void *user_data)
	{
		parseReal(ini, formal, store, user_data);

		float val;
		memcpy(&val, store, sizeof(val));
		if (val > 5.0f) val = 5.0f;
		memcpy(store, &val, sizeof(val));
	}

	static void _parseRealShroudClearingRange(INI *ini, void *formal, void *store, const void *user_data)
	{
		parseReal(ini, formal, store, user_data);

		float val = 999999.0f;
		memcpy(store, &val, sizeof(val));
	}

	static void _parseIntExperienceAward(INI *ini, void *formal, void *store, const void *user_data)
	{
		parseInt(ini, formal, store, user_data);

		int val = 0x10000;
		memcpy(store, &val, sizeof(val));
	}
};

bool LoadSingleAssetDat(FILE *f, int a2) { XCALL(0x00D38150); }

FILE *_fopen(const char *Filename, const char *Mode) { DSCALL(0x013593BC); }
int _fclose(FILE *File) { DSCALL(0x013593A0); }

bool _LoadSingleAssetDat(FILE *f, int a2)
{
	FILE *f2 = _fopen("asset2.dat", "rb");

	if (f2)
	{
		bool rv = LoadSingleAssetDat(f2, a2);

		_fclose(f2);

		if (!rv) return false;
	}

	return LoadSingleAssetDat(f, a2);
}

int parseNoShellMap(char **argv, int argc) { XCALL(0x004428FC); }
int parseMod(char **argv, int argc) { XCALL(0x00440070); }
int parseNoAudio(char **argv, int argc) { XCALL(0x00419CEF); }
int parseXRes(char **argv, int argc) { XCALL(0x00443C3E); }
int parseYRes(char **argv, int argc) { XCALL(0x00426CC4); }
int parseWin(char **argv, int argc) { XCALL(0x00401F2D); }
int parseScriptDebug2(char **argv, int argc) { XCALL(0x00430562); }
int parseScriptDebugLite(char **argv, int argc) { XCALL(0x00409DA4); }
int parseFullVersion(char **argv, int argc) { XCALL(0x00409C73); }
int parsePreferLocalFiles(char **argv, int argc) { XCALL(0x0042089C); }
int parseWatchdog(char **argv, int argc) { XCALL(0x00417490); }
int parseNoWatchdog(char **argv, int argc) { XCALL(0x0041FF9B); }
int parseNoMusic(char **argv, int argc) { XCALL(0x0043F6E3); }
int parseNoMuteOnFocusLoss(char **argv, int argc)
{
	if (TheWriteableGlobalData)
	{
		FIELD(bool, TheWriteableGlobalData, 0xA94) = false;
	}

	return 1;
}
int parseNoViewLimit(char **argv, int argc)
{
	if (TheWriteableGlobalData)
	{
		FIELD(bool, TheWriteableGlobalData, 0xCF2) = false;
	}

	return 1;
}
int parseHugeDump(char **argv, int argc) { XCALL(0x004043BD); }
int parseSelectTheUnselectable(char **argv, int argc)
{
	if (TheWriteableGlobalData)
	{
		FIELD(bool, TheWriteableGlobalData, 0xCF9) = false;
	}

	return 1;
}
int parseNoShroud(char **argv, int argc)
{
	if (TheWriteableGlobalData)
	{
		FIELD(bool, TheWriteableGlobalData, 0xCF4) = false;
	}

	return 1;
}
int parseShellMap(char **argv, int argc) { XCALL(0x00411EF0); }
int parseNoShellAnim(char **argv, int argc) { XCALL(0x0040B6C7); }
int parseBuildMapCache(char **argv, int argc) { XCALL(0x00438E56); }
int parseWinCursors(char **argv, int argc) { XCALL(0x0044335B); }
int parseNoLogo(char **argv, int argc)
{
	if (TheWriteableGlobalData)
	{
		FIELD(bool, TheWriteableGlobalData, 0xBB6) = false;
		FIELD(bool, TheWriteableGlobalData, 0xBB7) = true;
	}

	return 1;
}
int parseQuickStart(char **argv, int argc)
{
	parseNoLogo(argv, argc);
	parseNoShellMap(argv, argc);
	parseNoShellAnim(argv, argc);

	if (TheWriteableGlobalData)
	{
		FIELD(bool, TheWriteableGlobalData, 0x11FC) = true;
	}

	return 1;
}
int parseFile(char **argv, int argc) { XCALL(0x00462C50); }
int parseHelpText(char **argv, int argc) { XCALL(0x0043ED97); }
int parseNoHouseColor(char **argv, int argc)
{
	g_bHouseColor = false;

	return 1;
}
int parseStartingMoney(char **argv, int argc)
{
	if (argc > 1)
	{
		FIELD(int, TheWriteableGlobalData, 0x11F0) = atoi(argv[1]);

		g_flags |= 0x10;
	}

	return 2;
}
int parseFastGamePlay(char **argv, int argc)
{
	g_flags |= 0x8;

	if (TheWriteableGlobalData)
	{
		FIELD(bool, TheWriteableGlobalData, 0x11EC) = true;
	}

	return 1;
}
int parseSkipPreload(char **argv, int argc) { XCALL(0x0040975A); }
int parseLWTurbo(char **argv, int argc) { XCALL(0x0040FC95); }
int parseSkipMapUnroll(char **argv, int argc) { XCALL(0x004609A0); }

struct cmd_arg
{
	char *name;
	int (*parse)(char **, int);
};

cmd_arg params[] =
{
	{ "-noshellmap",				parseNoShellMap }, // original
	{ "-mod",						parseMod }, // original
	{ "-noaudio",					parseNoAudio }, // original
	{ "-xres",						parseXRes }, // original
	{ "-yres",						parseYRes }, // original
	{ "-win",						parseWin }, // original
	{ "-scriptDebug2",				parseScriptDebug2 }, // original
	{ "-scriptDebugLite",			parseScriptDebugLite }, // original
	{ "-fullVersion",				parseFullVersion }, // original
	{ "-preferLocalFiles",			parsePreferLocalFiles }, // original
	{ "-Watchdog",					parseWatchdog }, // original
	{ "-noWatchdog",				parseNoWatchdog }, // original
	{ "-noMusic",					parseNoMusic },
	{ "-noViewLimit",				parseNoViewLimit }, // no idea what it does
	{ "-noMuteOnFocusLoss",			parseNoMuteOnFocusLoss },
	{ "-hugedump",					parseHugeDump },
	{ "-selectTheUnselectable",		parseSelectTheUnselectable }, // doesn't seem to work
	{ "-noshroud",					parseNoShroud }, // doesn't seem to work
	{ "-shellmap",					parseShellMap },
	{ "-noShellAnim",				parseNoShellAnim },
	{ "-buildmapcache",				parseBuildMapCache },
	{ "-winCursors",				parseWinCursors }, // doesn't seem to work
	{ "-nologo",					parseNoLogo },
	{ "-quickstart",				parseQuickStart },
	{ "-file",						parseFile },
	{ "-helpText",					parseHelpText }, // no idea what it does
	{ "-nohousecolor",				parseNoHouseColor },
	{ "-StartingMoney",				parseStartingMoney }, // doesn't seem to work
	{ "-fastGamePlay",				parseFastGamePlay }, // doesn't seem to work
	{ "-skipPreload",				parseSkipPreload }, // no idea what it does
	{ "-lwturbo",					parseLWTurbo }, // no idea what it does
	{ "-skipmapunroll",				parseSkipMapUnroll },
};

ASM(fix_bFirewallEnabled)
{
	__asm
	{
		cmp byte ptr [ecx+0x270], 0
		jz ret_false

		cmp byte ptr [ecx+0x272], 0
		jz ret_false

		lea ecx, [ecx+0x268]
		mov eax, 0x00A90B70
		call eax
		test al, al
		jz ret_false
	}

	__asm inc al
	RET(0x0091DB64);

ret_false:;
	__asm xor al, al
	RET(0x0091DB64);
}

void patch()
{
	if (get_private_profile_bool("must_have", TRUE))
	{
		// disable needing the launcher
		Nop(0x00460598, 5 + 2 + 2 + 5 + 2);
		PatchByte(0x004605A8, 0xEB);

		// fix auto defeat
		BYTE sub_502240[] = { 0xB0, 0x01, 0xC3 };
		PatchBytes(0x00502240, sub_502240);

		// disable langdata.dat loading
		PatchString(0x010F5D64, "censored.dat");

		// actually use the bFirewallEnabled field
		InjectHook(0x0091DB4B, &fix_bFirewallEnabled, PATCH_JUMP);
		Nop(0x0091DB4B + 5, 1);
		PatchByte(0x0091DB66 + 1, 0x84);
	}

	if (get_private_profile_bool("params", TRUE))
	{
		// commandline arguments
		Patch(0x00463BE9 + 2, &params->name); // parseCommandLine()
		Patch(0x00463C27 + 2, &params->name); // parseCommandLine()
		Patch(0x00463C4B + 2, &params->parse); // parseCommandLine()

		// game only has 1 byte for sizeof, so we need more, thanks to the incremental build there's padding right before
		Nop(0x00463BD8, 2);
		Nop(0x00463BDA, 3);
		PatchByte(0x00463BDA + 3, 0x81);
		PatchByte(0x00463BDA + 4, 0xFB);
		Patch(0x00463BDA + 5, sizeof(params));
		Patch(0x00463C68 + 2, -142 - 3);

		// disable desync for some args
		Nop(0x00460A60, 5); // -noaudio
		Nop(0x00460A6B, 3); // -noaudio
		Nop(0x00460A6E, 5); // -noaudio

		Nop(0x00460A00, 7); // -noMusic
	}

	// doesnt't get centered properly, so don't enable this
	if (get_private_profile_bool("alt_splash_screen", FALSE))
	{
		Patch(0x0045F337 + 4, 640);
		Patch(0x0045F33F + 4, 480);

		Patch(0x004603BB + 1, "00000000.256"); // WinMain()
	}

	if (get_private_profile_bool("no_logo", TRUE))
	{
		Patch(0x0048537E + 2, 0xBB7); // GlobalData::GlobalData()
		Patch(0x00485388 + 2, 0xBB6); // GlobalData::GlobalData()
	}

	// forced_time_of_day
	{
		InjectHook(0x00ABED80, &GlobalData::_setTimeOfDay_forced); // W3DTerrainLogic::loadMap()
	}

	// and override if we want a random one
	if (get_private_profile_int("random_time_of_day", 0) != 0)
	{
		g_bUseRandomValue = get_private_profile_int("random_time_of_day", 0) == 1;

		InjectHook(0x00ABED80, &GlobalData::_setTimeOfDay_random); // W3DTerrainLogic::loadMap()
	}

	if (get_private_profile_bool("disable_asset_building", TRUE))
	{
		PatchJump(0x00D3864F, 0x00D3870E);
		Nop(0x00D3864F + 5);
	}

	if (get_private_profile_bool("asset2_dat", TRUE))
	{
		InjectHook(0x00D3898C, &_LoadSingleAssetDat); // InitializeAssetManager()
	}

	if (get_private_profile_bool("fire_sale", FALSE) || stristr(GetCommandLine(), "-dev"))
	{
		Patch(0x01088B94, &INI::_parseRealBuildTime);
		Patch(0x01088BA4, &INI::_parseIntBuildCost);
		Patch(0x01091254, &INI::_parseRealBuildTime);
		Patch(0x01091234, &INI::_parseUnsignedShortBuildCost);
		Patch(0x010A4524, &INI::_parseRealRebuildTimeSeconds);
		Patch(0x01091194, &INI::_parseRealShroudClearingRange);
	}

	if (get_private_profile_bool("xp_map", FALSE) || stristr(GetCommandLine(), "-exp"))
	{
		Patch(0x010EAB5C, &INI::_parseIntExperienceAward);
	}
}
}

namespace wb1
{
void patch()
{
	if (get_private_profile_bool("must_have", TRUE))
	{
		// get rid of eula popup
		Nop(0x004F73CA, 6);
		Nop(0x004F73D4, 5 + 3 + 6);
	}

	if (get_private_profile_bool("compression_none_default", TRUE))
	{
		Patch(0x00508480 + 1, 0); // CompressionManager::getPreferredCompression()

		Nop(0x004815E1, 6 + 1 + 5); // MapSettings::OnInitDialog()
		Nop(0x004815F4, 5 + 1 + 2 + 5 + 7 + 2); // MapSettings::OnInitDialog()
	}
}
}

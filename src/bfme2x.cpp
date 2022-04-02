#include <Windows.h>
#include <MMSystem.h>

#include "patch.h"
#include "bfme2x.h"

#define DEBUG_CRASHFIX // print some info to file about the bfme2x crashfix

namespace bfme2x
{
struct GlobalData;
extern GlobalData *&TheWriteableGlobalData;

extern uint32_t &g_flags;

extern bool &g_bHouseColor;
extern bool &g_bEditSystemCreateAHero;

struct AsciiString
{
	void *base;

	const char *str() { XCALL(0x004021D7); }
	void set(const char *str) { XCALL(0x004050E6); }
	int compare(const char *str) { XCALL(0x00406585); }
};

struct UnicodeString
{
	void *base;

	const wchar_t *str() { XCALL(0x004021E7); }
	void set(const wchar_t *str) { XCALL(0x0040514E); }
	int compare(const wchar_t *str) { XCALL(0x00406628); }
};

bool g_bDisableSkirmishAI;

struct SkirmishAIManager
{
	void update() { XCALL(0x006A96A0); }

	void _update()
	{
		if (!g_bDisableSkirmishAI) update();
	}
};

bool g_bDisableWOTRAI;

struct LivingWorldPlayer
{
	void SetAI() { XCALL(0x006E188C); }
	void SetDumb() { XCALL(0x006E1894); }

	void _SetAI()
	{
		if (g_bDisableWOTRAI) SetDumb();
		else SetAI();
	}
};

struct LivingWorldLogic
{
	void AddPlayer(void *player, bool isLocalPlayer, int isHumanOrNot, void *currGameSlot) { XCALL(0x006BB3B5); }

	void _AddPlayer(void *player, bool isLocalPlayer, int isHumanOrNot, void *currGameSlot)
	{
		printf("isDumb: %d, isLocalPlayer: %d, isHuman: %d, slotType: %d\n",
			FIELD(bool, player, 0x24),
			isLocalPlayer,
			isHumanOrNot == 0,
			currGameSlot ? FIELD(int, currGameSlot, 0x4) : -1
		);

		AddPlayer(player, isLocalPlayer, isHumanOrNot, currGameSlot);
	}
};

struct StringTable
{
	struct Entry
	{
		AsciiString label;
		UnicodeString value;
	};

	size_t num_entries;
	Entry *entries;
	Entry *sorted_entries;

	bool readSTR(char *strFile) { XCALL(0x006E6E69); }
	bool readCSF(char *csfFile) { XCALL(0x006E6C82); }

	void _fix()
	{
		Entry *good_campaign_entry = NULL;
		Entry *evil_campaign_entry = NULL;
		Entry *expansion1_campaign_entry = NULL;
		Entry *bonus_campaign_entry = NULL;

		for (size_t i = 0; i < num_entries; i++)
		{
			Entry *entry = &entries[i];

			if (!entry->label.compare("APT:GoodCampaign"))
			{
				good_campaign_entry = entry;
				continue;
			}

			if (!entry->label.compare("APT:EvilCampaign"))
			{
				evil_campaign_entry = entry;
				continue;
			}

			if (!entry->label.compare("APT:Expansion1Campaign"))
			{
				expansion1_campaign_entry = entry;
				continue;
			}

			if (!entry->label.compare("APT:BonusCampaign"))
			{
				bonus_campaign_entry = entry;
				continue;
			}
		}

		if (good_campaign_entry && expansion1_campaign_entry)
		{
			expansion1_campaign_entry->value.set(good_campaign_entry->value.str());
		}

		if (evil_campaign_entry && bonus_campaign_entry)
		{
			bonus_campaign_entry->value.set(evil_campaign_entry->value.str());
		}
	}

	bool _readSTR(char *strFile)
	{
		bool success = readSTR(strFile);

		if (success) _fix();

		return success;
	}

	bool _readCSF(char *csfFile)
	{
		bool success = readCSF(csfFile);

		if (success) _fix();

		return success;
	}
};

struct INI
{
	static void parseReal(INI *ini, void *obj, void *out, const void *null) { XCALL(0x0042ED00); }
	static void parseInt(INI *ini, void *obj, void *out, const void *null) { XCALL(0x0042EC5E); }
	static void parseUnsignedShort(INI *ini, void *obj, void *out, const void *null) { XCALL(0x0042EC11); }

	static void _parseRealBuildTime(INI *ini, void *obj, void *out, const void *null)
	{
		parseReal(ini, obj, out, null);

		float val = 5.0f;
		memcpy(out, &val, sizeof(val));
	}

	static void _parseIntBuildCost(INI *ini, void *obj, void *out, const void *null)
	{
		parseInt(ini, obj, out, null);

		int val = 10;
		memcpy(out, &val, sizeof(val));
	}

	static void _parseUnsignedShortBuildCost(INI *ini, void *obj, void *out, const void *null)
	{
		parseUnsignedShort(ini, obj, out, null);

		unsigned short val = 10;
		memcpy(out, &val, sizeof(val));
	}

	static void _parseRealShroudClearingRange(INI *ini, void *obj, void *out, const void *null)
	{
		parseReal(ini, obj, out, null);

		float val = 999999.0f;
		memcpy(out, &val, sizeof(val));
	}

	static void _parseIntExperienceAward(INI *ini, void *obj, void *out, const void *null)
	{
		parseInt(ini, obj, out, null);

		int val = 0x10000;
		memcpy(out, &val, sizeof(val));
	}
};

#ifdef DEBUG_CRASHFIX
FILE *crashfix_log;

void log_ebx(uint32_t ebx)
{
	if (!crashfix_log)
	{
		crashfix_log = fopen("crashfix_log.txt", "w");
	}

	if (crashfix_log)
	{
		fprintf(crashfix_log, "time: %08X, ebx: %08X\n", timeGetTime(), ebx);
	}
}
#endif

// this crashes because ebx can apparently be NULL even though it shouldn't be
// so here's a quick check if it is NULL ...
// TODO: so far this is untested
ASM(AIWallTactic_crashfix)
{
#ifdef DEBUG_CRASHFIX
	__asm
	{
		pusha
		push ebx
		call log_ebx
		pop ebx
		popa
	}
#endif

	__asm
	{
		test ebx, ebx
		jz locret
	}

	RET(0x009B6318);

locret:
	RET(0x009B63E7);
}

int parseNoShellMap(char **argv, int argc) { XCALL(0x007B9EC7); }
int parseMod(char **argv, int argc) { XCALL(0x007BADB9); }
int parseNoAudio(char **argv, int argc) { XCALL(0x007BA024); }
int parseXRes(char **argv, int argc) { XCALL(0x007BA104); }
int parseYRes(char **argv, int argc) { XCALL(0x007BA131); }
int parseWin(char **argv, int argc) { XCALL(0x007B9FC6); }
int parseScriptDebug2(char **argv, int argc) { XCALL(0x007BA1D4); }
int parseScriptDebugLite(char **argv, int argc) { XCALL(0x007BA1FB); }
int parseFullVersion(char **argv, int argc) { XCALL(0x007BA0B0); }
int parsePreferLocalFiles(char **argv, int argc) { XCALL(0x007B9EBE); }
int parseWatchdog(char **argv, int argc) { XCALL(0x007B9F36); }
int parseNoWatchdog(char **argv, int argc) { XCALL(0x007B9F5A); }
int parseRif(char **argv, int argc) { XCALL(0x007B9F3F); }
int parseFile(char **argv, int argc) { XCALL(0x007BACA4); }
int parseResumeGame(char **argv, int argc) { XCALL(0x007BACE3); }
int parseNoMusic(char **argv, int argc) { XCALL(0x007B9FEB); }
int parseNoViewLimit(char **argv, int argc)
{
	if (TheWriteableGlobalData)
	{
		FIELD(bool, TheWriteableGlobalData, 0xC67) = false;
	}

	return 1;
}
int parseHugeDump(char **argv, int argc) { XCALL(0x007BA3E0); }
int parseSelectTheUnselectable(char **argv, int argc)
{
	if (TheWriteableGlobalData)
	{
		FIELD(bool, TheWriteableGlobalData, 0xC6C) = false;
	}

	return 1;
}
int parseNoShroud(char **argv, int argc)
{
	if (TheWriteableGlobalData)
	{
		FIELD(bool, TheWriteableGlobalData, 0xC69) = false;
	}

	return 1;
}
int parseShellMap(char **argv, int argc)
{
	if (TheWriteableGlobalData)
	{
		if (argc > 1)
		{
			(FIELD(AsciiString, TheWriteableGlobalData, 0xAEC)).set(argv[1]);
		}
	}

	return 2;
}
int parseNoShellAnim(char **argv, int argc) { XCALL(0x007B9EF1); }
int parseBuildMapCache(char **argv, int argc) { XCALL(0x007BA252); }
int parseWinCursors(char **argv, int argc) { XCALL(0x007BA2A2); }
int parseNoLogo(char **argv, int argc)
{
	if (TheWriteableGlobalData)
	{
		FIELD(bool, TheWriteableGlobalData, 0xAF2) = false;
		FIELD(bool, TheWriteableGlobalData, 0xAF3) = true;
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
		FIELD(bool, TheWriteableGlobalData, 0x1110) = true;
	}

	return 1;
}
int parseHelpText(char **argv, int argc) { XCALL(0x007B9FD7); }
int parseNoHouseColor(char **argv, int argc)
{
	g_bHouseColor = false;

	return 1;
}
int parseNoMuteOnFocusLoss(char **argv, int argc)
{
	if (TheWriteableGlobalData)
	{
		FIELD(bool, TheWriteableGlobalData, 0x9C5) = false;
	}

	return 1;
}
int parseStartingMoney(char **argv, int argc)
{
	if (argc > 1)
	{
		FIELD(int, TheWriteableGlobalData, 0x1104) = atoi(argv[1]);

		g_flags |= 0x10;
	}

	return 2;
}
int parseFastGamePlay(char **argv, int argc)
{
	g_flags |= 0x8;

	if (TheWriteableGlobalData)
	{
		FIELD(bool, TheWriteableGlobalData, 0x1100) = true;
	}

	return 1;
}
int parseNoSkipPreload(char **argv, int argc) { XCALL(0x007BA748); }
int parseLWTurbo(char **argv, int argc) { XCALL(0x007BA77C); }
int parseSkipMapUnroll(char **argv, int argc) { XCALL(0x007B9F6E); }
int parseDisableSkirmishAI(char **argv, int argc)
{
	g_bDisableSkirmishAI = true;

	return 1;
}
int parseDisableWOTRAI(char **argv, int argc)
{
	g_bDisableWOTRAI = true;

	return 1;
}
int parseEditSystemCreateAHero(char **argv, int argc)
{
	g_bEditSystemCreateAHero = true;

	return 1;
}
int parseRandomSeed(char **argv, int argc) { XCALL(0x007BA795); }

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
	{ "-rif",						parseRif }, // original
	{ "-file",						parseFile }, // original
	{ "-resumeGame",				parseResumeGame }, // original
	{ "-noMusic",					parseNoMusic },
	{ "-noViewLimit",				parseNoViewLimit }, // no idea what it does
	{ "-hugedump",					parseHugeDump },
	{ "-selectTheUnselectable",		parseSelectTheUnselectable }, // doesn't seem to work
	{ "-noshroud",					parseNoShroud }, // doesn't seem to work
	{ "-shellmap",					parseShellMap },
	{ "-noShellAnim",				parseNoShellAnim },
	{ "-buildmapcache",				parseBuildMapCache },
	{ "-winCursors",				parseWinCursors }, // doesn't seem to work
	{ "-nologo",					parseNoLogo },
	{ "-quickstart",				parseQuickStart },
	{ "-helpText",					parseHelpText }, // no idea what it does
	{ "-nohousecolor",				parseNoHouseColor },
	{ "-noMuteOnFocusLoss",			parseNoMuteOnFocusLoss },
	{ "-StartingMoney",				parseStartingMoney }, // doesn't seem to work
	{ "-fastGamePlay",				parseFastGamePlay }, // doesn't seem to work
	{ "-noSkipPreload",				parseNoSkipPreload }, // no idea what it does
	{ "-lwturbo",					parseLWTurbo }, // no idea what it does
	{ "-skipmapunroll",				parseSkipMapUnroll },
	{ "-disableSkirmishAI",			parseDisableSkirmishAI },
	{ "-disableWOTRAI",				parseDisableWOTRAI },
	{ "-editSystemCreateAHero",		parseEditSystemCreateAHero },
	{ "-randomSeed",				parseRandomSeed }, // original
};

const size_t num_params = sizeof(params) / sizeof(params[0]);

ASM(fix_parseCommandLine)
{
	__asm
	{
		push num_params
		mov eax, 0x007BA7E1 // parseCommandLine()
		call eax
	}

	RET(0x007BAA5B);
}

void patch()
{
	if (get_private_profile_bool("params", TRUE))
	{
		// commandline arguments
		Patch(0x007BAA4B + 1, params); // ArchiveFileSystem::loadMods()
		Nop(0x007BAA54, 2); // get rid of push 16
		InjectHook(0x007BAA56, &fix_parseCommandLine, PATCH_JUMP);

		// disable desync for some args
		Nop(0x007BA029, 7); // -noaudio
		Nop(0x007B9FF0, 7); // -noMusic
	}

	// disable WinVerifyTrust check, fixes random version
	Nop(0x006444A1, 6 + 2 + 6 + 2);

	// disable needing the launcher
	Nop(0x00402C6E, 5 + 2 + 2 + 5 + 2);
	PatchByte(0x00402C7E, 0xEB);

	// fix auto defeat
	BYTE sub_63F7D8[] = { 0xB0, 0x01, 0xC3 };
	PatchBytes(0x0063F7D8, sub_63F7D8);

	// 0x009B6318 - fix crash when defeating enemy - apparently AIWallTactic related - AIWallTactic got added in rotwk ...
	// 0x01542190 (function, not potential crash addr) in WorldBuilder, but unfortunately no debug asserts there
	// is being called by AIWallTactic::update() (deduced from other vtable placements)
	if (get_private_profile_bool("old_crashfix", TRUE))
	{
		Nop(0x009B63E1, 6);
	}
	else
	{
		InjectHook(0x009B63E1, &AIWallTactic_crashfix);
		Nop(0x009B63E1 + 5);
	}

	// disable langdata.dat loading
	const char *langdata = "censored.dat";
	PatchBytes(0x00C2FE80, (unsigned char *)langdata, strlen(langdata) + 1);

	// don't check for bfme2 game.dat (so it can be renamed to RTS.exe)
	const char *LotRIcon = "LotRIcon.exe";
	Patch(0x00635FA4 + 1, LotRIcon);

	if (!is_bfme2x_202())
	{
		// unknown gamereplays patches, probably random number generation related
		BYTE sub_6D315D[] = { 0x53, 0x8B, 0x01, 0x31, 0xD2, 0xBB, 0x05, 0x84, 0x08, 0x08, 0xF7, 0xE3, 0x83, 0xC0, 0x01, 0x89, 0x01, 0x31, 0xD0, 0x5B, 0xC3 };
		PatchBytes(0x006D315D, sub_6D315D);

		BYTE sub_6D31D4[] = { 0x53, 0x52, 0xBB, 0xED, 0xFF, 0xFF, 0x7F, 0xF7, 0xE3, 0x89, 0x01, 0x89, 0x41, 0x04, 0x89, 0x41, 0x08, 0x89, 0x41, 0x0C, 0x89, 0x41, 0x10, 0x89, 0x41, 0x14, 0x5A, 0x5B, 0xC3 };
		PatchBytes(0x006D31D4, sub_6D31D4);
	}

	if (get_private_profile_bool("no_logo", TRUE))
	{
		Patch(0x0064309B + 2, 0xAF3); // GlobalData::GlobalData()
		Patch(0x006430A2 + 2, 0xAF2); // GlobalData::GlobalData()
	}

	if (get_private_profile_bool("bfme2_campaign", FALSE))
	{
		// the only difference between this and SubsystemLegendExpansion1 is LinearCampaign vs LinearCampaignExpansion1
		const char *SubsystemLegend = "Data\\INI\\Default\\SubsystemLegend.ini";
		Patch(0x0063AF50 + 1, SubsystemLegend); // GameEngine::init()

		Patch(0x0091BE8C + 1, 0x00C7CEB4); // change ANGMAR_CAMPAIGN to GOOD_CAMPAIGN
		Patch(0x0091BF0E + 1, 0x0091BDCA); // change ANGMAR_BONUS_CAMPAIGN to EVIL_CAMPAIGN

		// StringTable::init()
		InjectHook(0x006E7DBB, &StringTable::_readSTR);
		InjectHook(0x006E7DCF, &StringTable::_readCSF);
	}

	if (get_private_profile_bool("fire_sale", FALSE) || stristr(GetCommandLine(), "-dev"))
	{
		Patch(0x00C1082C, &INI::_parseRealBuildTime);
		Patch(0x00C1083C, &INI::_parseIntBuildCost);
		Patch(0x00DA404C, &INI::_parseRealBuildTime);
		Patch(0x00DA402C, &INI::_parseUnsignedShortBuildCost);
		Patch(0x00DA3F1C, &INI::_parseRealShroudClearingRange);
	}

	if (get_private_profile_bool("xp_map", FALSE))
	{
		Patch(0x00C11C04, &INI::_parseIntExperienceAward);
	}

	if (get_private_profile_bool("no_skirmish_ai", FALSE))
	{
		g_bDisableSkirmishAI = true;
	}

	// required for no_skirmish_ai and -disableSkirmishAI
	Patch(0x00C13E24, &SkirmishAIManager::_update);

	if (get_private_profile_bool("no_wotr_ai", FALSE))
	{
		g_bDisableWOTRAI = true;
	}

	// required for no_wotr_ai and -disableWOTRAI
	InjectHook(0x006B5EF1, &LivingWorldPlayer::_SetAI);
	InjectHook(0x006BB4B0, &LivingWorldPlayer::_SetAI);

	if (get_private_profile_bool("edit_system_cah", FALSE))
	{
		g_bEditSystemCreateAHero = true;
	}

	if (get_private_profile_bool("fix_wotr", FALSE))
	{
		NopTo(0x00841835, 0x00841844); // this just quick & dirty enables more humans to be enabled - BUT the game doesn't think 3-6 are humans!
		
		//PatchByte(0x008017BD + 3, 8); // num allowed humans in wotr?

		//BYTE wotr_patch[] = { 0xB8, 0x00, 0x00, 0x00, 0x00 };
		//PatchBytes(0x008454EF, wotr_patch);

		// LivingWorldCampaign::CreatePlayers()
		InjectHook(0x009330B7, &LivingWorldLogic::_AddPlayer);
		InjectHook(0x0093314F, &LivingWorldLogic::_AddPlayer);

		//PatchByte(0x0084592A, 0xEB);
	}

	// some notes of other old patches:
	// 0x0043D746: mov dl, 1 + nops for full crashdumps
	// 0x00A20DDC: jmp, for empty create a heroes? now we know it's a generic seralizer for a data format, so not good to patch around in there
}
}

namespace wb2x
{
void patch()
{
	if (get_private_profile_bool("compression_none_default", TRUE))
	{
		Patch(0x006C0240 + 1, 0); // CompressionManager::getPreferredCompression()

		Nop(0x0056BBD6, 3 + 1 + 5 + 5 + 1 + 3 + 5 + 3 + 4 + 2 + 2); // MapSettings::OnInitDialog()
	}

	// don't check for bfme2 game.dat (so it can be renamed to RTS.exe)
	const char *LotRIcon = "LotRIcon.exe";
	Patch(0x00CA4889 + 1, LotRIcon);
	Patch(0x00CA4892 + 1, LotRIcon);
	Patch(0x00CA48AF + 1, LotRIcon);
}
}
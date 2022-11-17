#include <Windows.h>
#include <MMSystem.h>

#include <ctime>

#include "patch.h"
#include "bfme2x.h"

namespace bfme2x
{
int GetGameLogicRandomValue(int min, int max, char *file, int line) { XCALL(0x006D328E); }

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

	void setTimeOfDay(TimeOfDay t) { XCALL(0x0064193F); }

	void _setTimeOfDay_forced(TimeOfDay t)
	{
		UINT u = get_dll_int("force_time_of_day", NONE);

		if (u >= MORNING && u <= NIGHT)
		{
			FIELD(TimeOfDay, this, 0x134) = (TimeOfDay)u;
			setTimeOfDay(FIELD(TimeOfDay, this, 0x134));
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

		if (get_dll_int("random_time_of_day", 0) == 1)
		{
			FIELD(TimeOfDay, this, 0x134) = r;
		}

		setTimeOfDay(FIELD(TimeOfDay, this, 0x134));
	}
};

extern GlobalData *&TheWritableGlobalData;

extern uint32_t &g_flags;

extern bool &g_bHouseColor;
extern bool &g_bEditSystemCreateAHero;

struct AsciiString
{
	void *data;

	AsciiString() { data = NULL; }
	~AsciiString() { releaseBuffer(); }

	void releaseBuffer() { XCALL(0x00435D50); }
	const char *str() { XCALL(0x004021D7); }
	void set(const char *str) { XCALL(0x004050E6); }
	int compare(const char *str) { XCALL(0x00406585); }
	void format(const char *fmt, ...) { XCALL(0x00437A90); }
};

struct UnicodeString
{
	void *data;

	UnicodeString() { data = NULL; }
	~UnicodeString() { releaseBuffer(); }

	void releaseBuffer() { XCALL(0x004367B0); }
	const wchar_t *str() { XCALL(0x004021E7); }
	void set(const wchar_t *str) { XCALL(0x0040514E); }
	int compare(const wchar_t *str) { XCALL(0x00406628); }
	void format(const wchar_t *fmt, ...) { XCALL(0x00ADF750); }
};

bool g_bDisableSkirmishAI;
int g_nIgnoreTicks;

struct SkirmishAIManager
{
	void update() { XCALL(0x006A96A0); }

	void _update()
	{
		if (!g_bDisableSkirmishAI) update();
	}

	void _update_ignore_ticks()
	{
		if (!g_bDisableSkirmishAI)
		{
			static int ticks = 0;

			if (ticks++ == g_nIgnoreTicks)
			{
				ticks = 0;

				_update();
			}
		}
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

	struct SortedEntry
	{
		Entry *label;
		Entry *value;
	};

	size_t num_entries;
	Entry *entries;
	SortedEntry *sorted_entries;

	bool init(char *strFile, char *csfFile, bool &success) { XCALL(0x006E7D9E); }
	bool readSTR(char *strFile) { XCALL(0x006E6E69); }
	bool readCSF(char *csfFile) { XCALL(0x006E6C82); }

	void _fix_expansion1_and_bonus()
	{
		Entry *evil_campaign_entry = NULL;
		Entry *good_campaign_entry = NULL;
		Entry *expansion1_campaign_entry = NULL;
		Entry *bonus_campaign_entry = NULL;

		Entry *evil_campaign_tooltip_entry = NULL;
		Entry *good_campaign_tooltip_entry = NULL;
		Entry *expansion1_campaign_tooltip_entry = NULL;
		Entry *bonus_campaign_tooltip_entry = NULL;

		for (size_t i = 0; i < num_entries; i++)
		{
			Entry *entry = &entries[i];

			if (!entry->label.compare("APT:EvilCampaign"))
			{
				evil_campaign_entry = entry;
				continue;
			}

			if (!entry->label.compare("APT:GoodCampaign"))
			{
				good_campaign_entry = entry;
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

			if (!entry->label.compare("ToolTip:OpeningMenu/EvilCampaignButton"))
			{
				evil_campaign_tooltip_entry = entry;
				continue;
			}

			if (!entry->label.compare("ToolTip:OpeningMenu/GoodCampaignButton"))
			{
				good_campaign_tooltip_entry = entry;
				continue;
			}

			if (!entry->label.compare("ToolTip:OpeningMenu/Expansion1Button"))
			{
				expansion1_campaign_tooltip_entry = entry;
				continue;
			}

			if (!entry->label.compare("ToolTip:OpeningMenu/BonusButton"))
			{
				bonus_campaign_tooltip_entry = entry;
				continue;
			}
		}

		if (evil_campaign_entry && expansion1_campaign_entry)
		{
			expansion1_campaign_entry->value.set(evil_campaign_entry->value.str());
		}

		if (good_campaign_entry && bonus_campaign_entry)
		{
			bonus_campaign_entry->value.set(good_campaign_entry->value.str());
		}

		if (evil_campaign_tooltip_entry && expansion1_campaign_tooltip_entry)
		{
			expansion1_campaign_tooltip_entry->value.set(evil_campaign_tooltip_entry->value.str());
		}

		if (good_campaign_tooltip_entry && bonus_campaign_tooltip_entry)
		{
			bonus_campaign_tooltip_entry->value.set(good_campaign_tooltip_entry->value.str());
		}
	}

	bool _readSTR(char *strFile)
	{
		bool b = readSTR(strFile);

		if (b) _fix_expansion1_and_bonus();

		return b;
	}

	bool _readCSF(char *csfFile)
	{
		bool b = readCSF(csfFile);

		if (b) _fix_expansion1_and_bonus();

		return b;
	}
};

struct INI
{
	static void parseReal(INI *ini, void *formal, void *store, const void *user_data) { XCALL(0x0042ED00); }
	static void parseInt(INI *ini, void *formal, void *store, const void *user_data) { XCALL(0x0042EC5E); }
	static void parseUnsignedInt(INI *ini, void *formal, void *store, const void *user_data) { XCALL(0x0042ECB2); }
	static void parseUnsignedShort(INI *ini, void *formal, void *store, const void *user_data) { XCALL(0x0042EC11); }
	static void parsePercentToReal(INI *ini, void *formal, void *store, const void *user_data) { XCALL(0x0042EEFA); }

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

	static void _parseUnsignedIntRespawnCost(INI *ini, void *formal, void *store, const void *user_data)
	{
		parseUnsignedInt(ini, formal, store, user_data);

		unsigned int val;
		memcpy(&val, store, sizeof(val));
		if (val > 10) val = 10;
		memcpy(store, &val, sizeof(val));
	}

	static void _parseIntRespawnTime(INI *ini, void *formal, void *store, const void *user_data)
	{
		parseInt(ini, formal, store, user_data);

		int val;
		memcpy(&val, store, sizeof(val));
		if (val > (5 * 1000)) val = (5 * 1000);
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

	static void _parsePercentToRealBuildCostReduction(INI *ini, void *formal, void *store, const void *user_data)
	{
		parsePercentToReal(ini, formal, store, user_data);

		float val = get_dll_float("brutal_difficulty_build_cost_reduction", 15.0f) / 100.0f;
		memcpy(store, &val, sizeof(val));
	}

	static void _parsePercentToRealBuildTimeReduction(INI *ini, void *formal, void *store, const void *user_data)
	{
		parsePercentToReal(ini, formal, store, user_data);

		float val = get_dll_float("brutal_difficulty_build_time_reduction", 15.0f) / 100.0f;
		memcpy(store, &val, sizeof(val));
	}
};

ASM(fix_handicap_list_init_value)
{
	__asm
	{
		lea esi, [ecx+eax*4+0x354]
		mov edi, -100
	}

	RET(0x00840BB8);
}

const wchar_t *handicap_string_fmt = L"+%d%%";

ASM(fix_handicap_list_string)
{
	__asm
	{
		cmp edi, 0
		jg positive
		jmp negative

	positive:
		push handicap_string_fmt
		jmp locret

	negative:
		push 0x00C53FB8
		jmp locret
	}

locret:;
	RET(0x00840BE8);
}

struct Object
{
	bool addAttributeModifierToPool(AsciiString &, int) { XCALL(0x0068F1A8); }

	void _fixHandicap(int handicap)
	{
		if (handicap != 0 && !(-handicap % 5))
		{
			if (handicap >= -100 && handicap < 0)
			{
				AsciiString str;
				str.format("HandicapPercent%d", -handicap);
				addAttributeModifierToPool(str, -1);
			}
			else if (handicap > 0 && handicap <= 100)
			{
				AsciiString str;
				str.format("BonusPercent%d", handicap);
				addAttributeModifierToPool(str, -1);
			}
		}
	}
};

ASM(fix_handicap_callFixHandicap)
{
	__asm
	{
		push eax
		mov ecx, esi
		call Object::_fixHandicap
	}

	RET(0x00694130);
}

ASM(fix_handicap_LANAPI__OnGameOptions)
{
	__asm
	{
		test eax, eax
		jnz loc_64A44B

		cmp edi, -100
		jl loc_64A3F7

		cmp edi, 100
		jg loc_64A3F7
	}

	RET(0x0064A43A);

loc_64A3F7:;
	RET(0x0064A3F7);

loc_64A44B:;
	RET(0x0064A44B);
}

ASM(fix_handicap_AptOnlineCustomMatch__ProcessGameSpyResponses)
{
	__asm
	{
		test eax, eax
		jnz loc_9ADBF9

		cmp edi, -100
		jl loc_9ADA43

		cmp edi, 100
		jg loc_9ADA43
	}

	RET(0x009ADBE1);

loc_9ADA43:;
	RET(0x009ADA43);

loc_9ADBF9:;
	RET(0x009ADBF9);
}

int _atoi(char *) { DSCALL(0x00BD0628); }

ASM(fix_handicap_ParseAsciiStringToGameInfo)
{
	__asm
	{
		call _atoi
		pop ecx
		cmp eax, -100
		jl loc_8036B3

		cmp eax, 100
		jg loc_8036B3
	}

	RET(0x00803627);

loc_8036B3:;
	RET(0x008036B3);
}

bool LoadSingleAssetDat(FILE *f, int a2) { XCALL(0x0052C577); }

FILE *_fopen(const char *Filename, const char *Mode) { DSCALL(0x00BD0554); }
int _fclose(FILE *File) { DSCALL(0x00BD0560); }

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

struct Version
{
	int version1;
	int version2;
	int version3;
	int version4;
	AsciiString id;
	AsciiString machine;
	AsciiString user;
	AsciiString guid;
	AsciiString time;
	AsciiString date;
	AsciiString config;
	bool b;

	// in this function we only return the bfme2 green colors, not the rotwk blue colors, but they are originally already present in bfme2
	unsigned int _getGameColor(Version *other, bool force_wrong_version)
	{
		bool same_version = false;

		if (!force_wrong_version && version1 == other->version1 && version2 == other->version2 && version3 == other->version3 && version4 == other->version4)
		{
			same_version = true;
		}

		return same_version ? 0xFF7AAB44 : 0xFF3D5522;
	}
};

struct AIWallTactic
{
	// this crashes because it seems like it tries to iterate some list (0xCC - 0xD0), but 0xCC is NULL
	// i assume this is because "this" already got destroyed, because it only happens when ai gets killed
	// so first, we just log the pointers and time each time this is called ...
	bool crashes()
	{
		static FILE *log;

		if (!log)
		{
			log = fopen("aiwalltactic_crashfix_log.txt", "a");

			if (log) setbuf(log, NULL);
		}

		if (log)
		{
			fprintf(log, "time: %d, 0xCC: %p, 0xD0: %p\n", std::time(nullptr), FIELD(void *, this, 0xCC), FIELD(void *, this, 0xD0));
		}

		return false;
	}
};

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
	if (TheWritableGlobalData)
	{
		FIELD(bool, TheWritableGlobalData, 0xC67) = false;
	}

	return 1;
}
int parseHugeDump(char **argv, int argc) { XCALL(0x007BA3E0); }
int parseSelectTheUnselectable(char **argv, int argc)
{
	if (TheWritableGlobalData)
	{
		FIELD(bool, TheWritableGlobalData, 0xC6C) = true;
	}

	return 1;
}
int parseNoShroud(char **argv, int argc)
{
	if (TheWritableGlobalData)
	{
		FIELD(bool, TheWritableGlobalData, 0xC69) = false;
	}

	return 1;
}
int parseShellMap(char **argv, int argc)
{
	if (TheWritableGlobalData)
	{
		if (argc > 1)
		{
			(FIELD(AsciiString, TheWritableGlobalData, 0xAEC)).set(argv[1]);
		}
	}

	return 2;
}
int parseNoShellAnim(char **argv, int argc) { XCALL(0x007B9EF1); }
int parseBuildMapCache(char **argv, int argc) { XCALL(0x007BA252); }
int parseWinCursors(char **argv, int argc) { XCALL(0x007BA2A2); }
int parseNoLogo(char **argv, int argc)
{
	if (TheWritableGlobalData)
	{
		FIELD(bool, TheWritableGlobalData, 0xAF2) = false;
		FIELD(bool, TheWritableGlobalData, 0xAF3) = true;
	}

	return 1;
}
int parseQuickStart(char **argv, int argc)
{
	parseNoLogo(argv, argc);
	parseNoShellMap(argv, argc);
	parseNoShellAnim(argv, argc);

	if (TheWritableGlobalData)
	{
		FIELD(bool, TheWritableGlobalData, 0x1110) = true;
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
	if (TheWritableGlobalData)
	{
		FIELD(bool, TheWritableGlobalData, 0x9C5) = false;
	}

	return 1;
}
int parseStartingMoney(char **argv, int argc)
{
	if (argc > 1)
	{
		FIELD(int, TheWritableGlobalData, 0x1104) = atoi(argv[1]);

		g_flags |= 0x10;
	}

	return 2;
}
int parseFastGamePlay(char **argv, int argc)
{
	g_flags |= 0x8;

	if (TheWritableGlobalData)
	{
		FIELD(bool, TheWritableGlobalData, 0x1100) = true;
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
	{ "-noshellmap",                parseNoShellMap }, // original
	{ "-mod",                       parseMod }, // original
	{ "-noaudio",                   parseNoAudio }, // original
	{ "-xres",                      parseXRes }, // original
	{ "-yres",                      parseYRes }, // original
	{ "-win",                       parseWin }, // original
	{ "-scriptDebug2",              parseScriptDebug2 }, // original
	{ "-scriptDebugLite",           parseScriptDebugLite }, // original
	{ "-fullVersion",               parseFullVersion }, // original
	{ "-preferLocalFiles",          parsePreferLocalFiles }, // original
	{ "-Watchdog",                  parseWatchdog }, // original
	{ "-noWatchdog",                parseNoWatchdog }, // original
	{ "-rif",                       parseRif }, // original
	{ "-file",                      parseFile }, // original
	{ "-resumeGame",                parseResumeGame }, // original
	{ "-noMusic",                   parseNoMusic },
	{ "-noViewLimit",               parseNoViewLimit }, // no idea what it does
	{ "-hugedump",                  parseHugeDump },
	{ "-selectTheUnselectable",     parseSelectTheUnselectable }, // doesn't seem to work
	{ "-noshroud",                  parseNoShroud }, // doesn't seem to work
	{ "-shellmap",                  parseShellMap },
	{ "-noShellAnim",               parseNoShellAnim },
	{ "-buildmapcache",             parseBuildMapCache },
	{ "-winCursors",                parseWinCursors }, // doesn't seem to work
	{ "-nologo",                    parseNoLogo },
	{ "-quickstart",                parseQuickStart },
	{ "-helpText",                  parseHelpText }, // no idea what it does
	{ "-nohousecolor",              parseNoHouseColor },
	{ "-noMuteOnFocusLoss",         parseNoMuteOnFocusLoss },
	{ "-StartingMoney",             parseStartingMoney }, // doesn't seem to work
	{ "-fastGamePlay",              parseFastGamePlay }, // doesn't seem to work
	{ "-noSkipPreload",             parseNoSkipPreload }, // no idea what it does
	{ "-lwturbo",                   parseLWTurbo }, // no idea what it does
	{ "-skipmapunroll",             parseSkipMapUnroll },
	{ "-disableSkirmishAI",         parseDisableSkirmishAI },
	{ "-disableWOTRAI",             parseDisableWOTRAI },
	{ "-editSystemCreateAHero",     parseEditSystemCreateAHero },
	{ "-randomSeed",                parseRandomSeed }, // original
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
	if (get_dll_bool("must_have", TRUE))
	{
		// disable WinVerifyTrust check, fixes random version
		Nop(0x006444A1, 6 + 2 + 6 + 2);

		// disable needing the launcher
		Nop(0x00402C6E, 5 + 2 + 2 + 5 + 2);
		PatchByte(0x00402C7E, 0xEB);

		// fix auto defeat
		BYTE sub_63F7D8[] = { 0xB0, 0x01, 0xC3 };
		PatchBytes(0x0063F7D8, sub_63F7D8);

		// disable langdata.dat loading
		PatchString(0x00C2FE80, "censored.dat");

		// don't check for bfme2 game.dat (so it can be renamed to RTS.exe)
		Patch(0x00635FA4 + 1, "LotRIcon.exe");
	}

	if (get_dll_bool("params", TRUE))
	{
		// commandline arguments
		Patch(0x007BAA4B + 1, params); // ArchiveFileSystem::loadMods()
		Nop(0x007BAA54, 2); // get rid of push 16
		InjectHook(0x007BAA56, &fix_parseCommandLine, PATCH_JUMP);

		// disable desync for some args
		Nop(0x007BA029, 7); // -noaudio
		Nop(0x007B9FF0, 7); // -noMusic
	}

	// enables handicap from -100% to +100% (Bonus instead of Handicap)
	if (get_dll_bool("handicap", TRUE))
	{
		// first we fix the dropdown list
		// AptMpGameSetup::PopulateHandicapCombo()
		InjectHook(0x00840BAF, &fix_handicap_list_init_value, PATCH_JUMP); // init to -100
		PatchByte(0x00840C22 + 1, 0xC7); // += 5 instead of -=5
		PatchByte(0x00840C25 + 2, 100); // check <= 100
		PatchByte(0x00840C28, 0x7E); // check <= 100
		InjectHook(0x00840BE3, &fix_handicap_list_string, PATCH_JUMP);

		// then we fix the parsing
		// Object::initObject()
		InjectHook(0x006940E4, &fix_handicap_callFixHandicap, PATCH_JUMP);

		// then we fix the network transmission of this value, 4 functions check that it's in range of -100 to 0 ...
		// LANAPI::OnGameOptions()
		InjectHook(0x0064A42D, &fix_handicap_LANAPI__OnGameOptions, PATCH_JUMP);

		// AptOnlineCustomMatch::ProcessGameSpyResponses()
		InjectHook(0x009ADBCC, &fix_handicap_AptOnlineCustomMatch__ProcessGameSpyResponses, PATCH_JUMP);

		// ByteBufferToGameInfo()
		PatchByte(0x0084B57E + 3, 100);
		PatchByte(0x0084B857 + 3, 100);

		// ParseAsciiStringToGameInfo()
		InjectHook(0x0080360F, &fix_handicap_ParseAsciiStringToGameInfo, PATCH_JUMP);
	}

	// 0x009B6318 - fix crash when defeating enemy - apparently AIWallTactic related - AIWallTactic got added in rotwk ...
	// 0x01542190 (function, not potential crash addr) in WorldBuilder, but unfortunately no debug asserts there
	// is being called by AIWallTactic::update() (deduced from other vtable placements)
	if (get_dll_int("aiwalltactic_crashfix", 1) != 0)
	{
		if (get_dll_int("aiwalltactic_crashfix", 1) == 1)
		{
			BYTE sub_9B62F6[] = { 0x30, 0xC0, 0xC3 };
			PatchBytes(0x009B62F6, sub_9B62F6);
		}
		else
		{
			InjectHook(0x009B7186, &AIWallTactic::crashes);
		}
	}

	if (get_dll_bool("gamereplays_rand", TRUE) && !is_bfme2x_202())
	{
		// called by i.e. GetGameLogicRandomValue, seems to be a custom rand function
		BYTE sub_6D315D[] = { 0x53, 0x8B, 0x01, 0x31, 0xD2, 0xBB, 0x05, 0x84, 0x08, 0x08, 0xF7, 0xE3, 0x83, 0xC0, 0x01, 0x89, 0x01, 0x31, 0xD0, 0x5B, 0xC3 };
		PatchBytes(0x006D315D, sub_6D315D);

		// called by InitRandom, inits a seed array used by above function
		BYTE sub_6D31D4[] = { 0x53, 0x52, 0xBB, 0xED, 0xFF, 0xFF, 0x7F, 0xF7, 0xE3, 0x89, 0x01, 0x89, 0x41, 0x04, 0x89, 0x41, 0x08, 0x89, 0x41, 0x0C, 0x89, 0x41, 0x10, 0x89, 0x41, 0x14, 0x5A, 0x5B, 0xC3 };
		PatchBytes(0x006D31D4, sub_6D31D4);
	}

	if (get_dll_bool("no_logo", TRUE))
	{
		Patch(0x0064309B + 2, 0xAF3); // GlobalData::GlobalData()
		Patch(0x006430A2 + 2, 0xAF2); // GlobalData::GlobalData()
	}

	// forced_time_of_day
	{
		InjectHook(0x00462A4D, &GlobalData::_setTimeOfDay_forced); // W3DTerrainLogic::loadMap()
	}

	// and override if we want a random one
	if (get_dll_int("random_time_of_day", 0) != 0)
	{
		InjectHook(0x00462A4D, &GlobalData::_setTimeOfDay_random); // W3DTerrainLogic::loadMap()
	}

	if (get_dll_bool("disable_asset_building", TRUE))
	{
		PatchByte(0x00D9B2A0, false); // assetCacheBuilder.exe
		PatchByte(0x00D9B2A1, false); // TextureAssetBuilder.exe
		PatchByte(0x00D9B2A2, false); // ShaderAssetBuilder.exe
	}

	if (get_dll_bool("asset2_dat", TRUE))
	{
		InjectHook(0x0052CC5B, &_LoadSingleAssetDat); // InitializeAssetManager()
	}

	{
		WIN32_FIND_DATA info;
		HANDLE handle = FindFirstFile("EnglishSplash.jpg", &info);

		if (info.nFileSizeLow == 98654) // this means we are using the bfme2 splash screen (English)
		{
			InjectHook(0x0084856C, &Version::_getGameColor); // AptLanLobby::ShowSortedGameList()
			InjectHook(0x009A8CEE, &Version::_getGameColor); // AptOnlineCustomMatch::InsertGameInListbox()

			// disable WOTR and CreateAHero promo screens
			BYTE sub_6E5E72[] = { 0x31, 0xC0, 0x40, 0xC2, 0x04, 0x00 };
			PatchBytes(0x006E5E72, sub_6E5E72);
		}

		FindClose(handle);
	}

	if (get_dll_bool("bfme2_campaign", FALSE))
	{
		// the only difference between this and SubsystemLegendExpansion1.ini is LinearCampaign vs LinearCampaignExpansion1
		Patch(0x0063AF50 + 1, "Data\\INI\\Default\\SubsystemLegend.ini"); // GameEngine::init()

		InjectHook(0x0091C3C9, 0x0091C215); // change ANGMAR_CAMPAIGN to EVIL_CAMPAIGN
		InjectHook(0x0091C3D7, 0x0091C1C8); // change ANGMAR_BONUS_CAMPAIGN to GOOD_CAMPAIGN

		InjectHook(0x0091C3C9, 0x0091C215); // change ANGMAR_CAMPAIGN to EVIL_CAMPAIGN
		InjectHook(0x0091C2AF, 0x0091C1C8); // change ANGMAR_BONUS_CAMPAIGN to GOOD_CAMPAIGN

		// StringTable::init()
		InjectHook(0x006E7DBB, &StringTable::_readSTR);
		InjectHook(0x006E7DCF, &StringTable::_readCSF);

		// forces Bonus Campaign button to be available
		BYTE sub_6E5E16[] = { 0xB0, 0x01, 0xC3 };
		PatchBytes(0x006E5E16, sub_6E5E16);
	}

	if (get_dll_bool("no_skirmish_ai", FALSE))
	{
		g_bDisableSkirmishAI = true;
	}

	// required for no_skirmish_ai and -disableSkirmishAI
	{
		Patch(0x00C13E24, &SkirmishAIManager::_update);
	}

	if (get_dll_bool("no_wotr_ai", FALSE))
	{
		g_bDisableWOTRAI = true;
	}

	// required for no_wotr_ai and -disableWOTRAI
	{
		InjectHook(0x006B5EF1, &LivingWorldPlayer::_SetAI);
		InjectHook(0x006BB4B0, &LivingWorldPlayer::_SetAI);
	}

	if (get_dll_bool("brutal_difficulty_cheats", FALSE))
	{
		Patch(0x00C13948, &INI::_parsePercentToRealBuildCostReduction);
		Patch(0x00C13958, &INI::_parsePercentToRealBuildTimeReduction);
	}

	if (get_dll_bool("fire_sale", FALSE) || stristr(GetCommandLine(), "-dev"))
	{
		Patch(0x00C1082C, &INI::_parseRealBuildTime);
		Patch(0x00C1083C, &INI::_parseIntBuildCost);
		Patch(0x00DA404C, &INI::_parseRealBuildTime);
		Patch(0x00DA402C, &INI::_parseUnsignedShortBuildCost);
		Patch(0x00C56B2C, &INI::_parseRealRebuildTimeSeconds);
		Patch(0x00DA3F1C, &INI::_parseRealShroudClearingRange);
		InjectHook(0x008B3E6B, &INI::_parseUnsignedIntRespawnCost); // RespawnUpdate::iniParseDefaultRule()
		InjectHook(0x008B3EC4, &INI::_parseIntRespawnTime); // RespawnUpdate::iniParseDefaultRule()
		InjectHook(0x008B411F, &INI::_parseUnsignedIntRespawnCost); // RespawnUpdate::iniParseNewRuleForLevel()
		InjectHook(0x008B4165, &INI::_parseIntRespawnTime); // RespawnUpdate::iniParseNewRuleForLevel()

		g_nIgnoreTicks = get_dll_int("ignore_ticks", 60);
		if (g_nIgnoreTicks <= 0) g_nIgnoreTicks = 1;

		if (g_nIgnoreTicks != 1) Patch(0x00C13E24, &SkirmishAIManager::_update_ignore_ticks);
	}

	if (get_dll_bool("xp_map", FALSE) || stristr(GetCommandLine(), "-exp"))
	{
		Patch(0x00C11C04, &INI::_parseIntExperienceAward);
	}

	if (get_dll_bool("no_shroud", FALSE) || stristr(GetCommandLine(), "-noshroud"))
	{
		Patch(0x00DA3F1C, &INI::_parseRealShroudClearingRange);
	}

	if (get_dll_bool("edit_system_cah", FALSE))
	{
		g_bEditSystemCreateAHero = true;
	}

	if (get_dll_bool("fix_wotr", FALSE))
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
bool LoadSingleAssetDat(FILE *f, int a2) { XCALL(0x009D7270); }

FILE *_fopen(const char *Filename, const char *Mode) { DSCALL(0x022F51A0); }
int _fclose(FILE *File) { DSCALL(0x022F51A8); }

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

void patch()
{
	if (get_dll_bool("must_have", TRUE))
	{
		// don't check for bfme2 game.dat (so it can be renamed to RTS.exe)
		Patch(0x00CA4889 + 1, "LotRIcon.exe");
		Patch(0x00CA4892 + 1, "LotRIcon.exe");
		Patch(0x00CA48AF + 1, "LotRIcon.exe");
	}

	if (get_dll_bool("asset2_dat", TRUE))
	{
		InjectHook(0x009D8027, &_LoadSingleAssetDat); // InitializeAssetManager()
	}

	if (get_dll_bool("compression_none_default", TRUE))
	{
		Patch(0x006C0240 + 1, 0); // CompressionManager::getPreferredCompression()

		Nop(0x0056BBD6, 3 + 1 + 5 + 5 + 1 + 3 + 5 + 3 + 4 + 2 + 2); // MapSettings::OnInitDialog()
	}
}
}
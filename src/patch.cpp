#include <Windows.h>
#include <MMSystem.h>

#include "patch.h"

#define DEBUG_CRASHFIX // print some info to file about the crashfix

#define is_bfme1() (*(uint32_t *)0x004600F0 == 0x6AEC8B55)
#define is_wb1() (*(uint32_t *)0x00E2B0AC == 0x0010D9E9)

#define is_bfme2x() (*(uint32_t *)0x004027F7 == 0xB6F7B4B8)
#define is_wb2x() (*(uint32_t *)0x018463E7 == 0x000FF6E9)

struct cmd_arg
{
	char *name;
	int (*parse)(char **, int);
};

namespace bfme1
{
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

int parseNoLogo(char **argv, int argc)
{
	if (TheWriteableGlobalData)
	{
		FIELD(bool, TheWriteableGlobalData, 0xBB6) = false;
		FIELD(bool, TheWriteableGlobalData, 0xBB7) = true;
	}

	return 1;
}

int parseBuildMapCache(char **argv, int argc) { XCALL(0x00438E56); }

cmd_arg params[] =
{
	{ "-noshellmap",				parseNoShellMap },
	{ "-mod",						parseMod },
	{ "-noaudio",					parseNoAudio },
	{ "-xres",						parseXRes },
	{ "-yres",						parseYRes },
	{ "-win",						parseWin },
	{ "-scriptDebug2",				parseScriptDebug2 },
	{ "-scriptDebugLite",			parseScriptDebugLite },
	{ "-fullVersion",				parseFullVersion },
	{ "-preferLocalFiles",			parsePreferLocalFiles },
	{ "-Watchdog",					parseWatchdog },
	{ "-noWatchdog",				parseNoWatchdog },

	{ "-noMusic",					parseNoMusic },
	{ "-nologo",					parseNoLogo },
	{ "-buildmapcache",				parseBuildMapCache },
};

void patch()
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

	// disable needing the launcher
	Nop(0x00460598, 5 + 2 + 2 + 5 + 2);
	PatchByte(0x004605A8, 0xEB);

	// fix auto defeat
	BYTE sub_502240[] = { 0xB0, 0x01, 0xC3 };
	PatchBytes(0x00502240, sub_502240);

	// disable langdata.dat loading
	const char *langdata = "censored.dat";
	PatchBytes(0x010F5D64, (unsigned char *)langdata, strlen(langdata) + 1);

	if (get_private_profile_bool("no_logo", TRUE))
	{
		Patch(0x0048537E + 2, 0xBB7); // GlobalData::GlobalData()
		Patch(0x00485388 + 2, 0xBB6); // GlobalData::GlobalData()
	}

	// doesnt't get centered properly
	if (get_private_profile_bool("alt_splash_screen", FALSE))
	{
		Patch(0x0045F337 + 4, 640);
		Patch(0x0045F33F + 4, 480);

		const char *splash = "00000000.256";
		Patch(0x004603BB + 1, splash); // WinMain()
	}
}
}

namespace wb1
{
void patch()
{
	if (get_private_profile_bool("compression_none_default", TRUE))
	{
		Patch(0x00508480 + 1, 0); // CompressionManager::getPreferredCompression()

		Nop(0x004815E1, 6 + 1 + 5); // MapSettings::OnInitDialog()
		Nop(0x004815F4, 5 + 1 + 2 + 5 + 7 + 2); // MapSettings::OnInitDialog()
	}

	// get rid of eula popup
	Nop(0x004F73CA, 6);
	Nop(0x004F73D4, 5 + 3 + 6);
}
}

namespace bfme2x
{
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
int parseRandomSeed(char **argv, int argc) { XCALL(0x007BA795); }

int parseNoMusic(char **argv, int argc) { XCALL(0x007B9FEB); }

int parseNoLogo(char **argv, int argc)
{
	if (TheWriteableGlobalData)
	{
		FIELD(bool, TheWriteableGlobalData, 0xAF2) = false;
		FIELD(bool, TheWriteableGlobalData, 0xAF3) = true;
	}

	return 1;
}

int parseBuildMapCache(char **argv, int argc) { XCALL(0x007BA252); }

int parseEditSystemCreateAHero(char **argv, int argc)
{
	g_bEditSystemCreateAHeroes = true;

	return 1;
}

cmd_arg params[] =
{
	{ "-noshellmap",				parseNoShellMap },
	{ "-mod",						parseMod },
	{ "-noaudio",					parseNoAudio },
	{ "-xres",						parseXRes },
	{ "-yres",						parseYRes },
	{ "-win",						parseWin },
	{ "-scriptDebug2",				parseScriptDebug2 },
	{ "-scriptDebugLite",			parseScriptDebugLite },
	{ "-fullVersion",				parseFullVersion },
	{ "-preferLocalFiles",			parsePreferLocalFiles },
	{ "-Watchdog",					parseWatchdog },
	{ "-noWatchdog",				parseNoWatchdog },
	{ "-rif",						parseRif },
	{ "-file",						parseFile },
	{ "-resumeGame",				parseResumeGame },
	{ "-randomSeed",				parseRandomSeed },

	{ "-noMusic",					parseNoMusic },
	{ "-nologo",					parseNoLogo },
	{ "-buildmapcache",				parseBuildMapCache },
	{ "-editSystemCreateAHero",		parseEditSystemCreateAHero },
};

const size_t num_params = sizeof(params) / sizeof(params[0]);

ASM(fix_parseCommandLine)
{
	__asm
	{
		push num_params
		mov eax, 0x007BA7E1
		call eax
	}

	RET(0x007BAA5B);
}

void patch()
{
	// commandline arguments
	Patch(0x007BAA4B + 1, params); // ArchiveFileSystem::loadMods()
	Nop(0x007BAA54, 2); // get rid of push 16
	InjectHook(0x007BAA56, &fix_parseCommandLine, PATCH_JUMP);

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

	// unknown gamereplays patches, probably random number generation related
	BYTE sub_6D315D[] = { 0x53, 0x8B, 0x01, 0x31, 0xD2, 0xBB, 0x05, 0x84, 0x08, 0x08, 0xF7, 0xE3, 0x83, 0xC0, 0x01, 0x89, 0x01, 0x31, 0xD0, 0x5B, 0xC3 };
	PatchBytes(0x006D315D, sub_6D315D);

	BYTE sub_6D31D4[] = { 0x53, 0x52, 0xBB, 0xED, 0xFF, 0xFF, 0x7F, 0xF7, 0xE3, 0x89, 0x01, 0x89, 0x41, 0x04, 0x89, 0x41, 0x08, 0x89, 0x41, 0x0C, 0x89, 0x41, 0x10, 0x89, 0x41, 0x14, 0x5A, 0x5B, 0xC3 };
	PatchBytes(0x006D31D4, sub_6D31D4);

	if (get_private_profile_bool("no_logo", TRUE))
	{
		Patch(0x0064309B + 2, 0xAF3); // GlobalData::GlobalData()
		Patch(0x006430A2 + 2, 0xAF2); // GlobalData::GlobalData()
	}

	if (get_private_profile_bool("fix_wotr", FALSE))
	{
		NopTo(0x00841835, 0x00841844); // this just quick & dirty enables more humans to be enabled - BUT the game doesn't think 3-6 are humans!
		
		//Nop(0x00801543, 2);

		//PatchByte(0x008017BD + 3, 8); // num allowed humans in wotr?

		//BYTE wotr_patch[] = { 0xB8, 0x00, 0x00, 0x00, 0x00 };
		//PatchBytes(0x008454EF, wotr_patch);
	}

	// some notes of other patches:
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

void patch()
{
	if (is_bfme1())
	{
		bfme1::patch();
	}
	else if (is_wb1())
	{
		wb1::patch();
	}
	else if (is_bfme2x())
	{
		bfme2x::patch();
	}
	else if (is_wb2x())
	{
		wb2x::patch();
	}
}

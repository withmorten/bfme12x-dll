#pragma once

#include <patcher.h>

#include "dllmain.h"

namespace bfme1
{
struct GlobalData;
extern GlobalData *&TheWriteableGlobalData;

extern uint32_t &g_flags;

extern bool &g_bHouseColor;
};

namespace wb1
{
};

namespace bfme2
{
};

namespace wb2
{
};

namespace bfme2x
{
struct GlobalData;
extern GlobalData *&TheWriteableGlobalData;

extern uint32_t &g_flags;

extern bool &g_bHouseColor;
extern bool &g_bEditSystemCreateAHero;

struct AsciiString
{
	void set(const char *) { XCALL(0x004050E6); }
};

struct SkirmishAIManager
{
	static bool s_bDisableSkirmishAI;

	void update() { XCALL(0x006A96A0); }

	void _update()
	{
		if (!s_bDisableSkirmishAI) update();
	}
};

struct LivingWorldPlayer
{
	static bool s_bDisableWOTRAI;

	void SetAI() { XCALL(0x006E188C); }
	void SetHuman() { XCALL(0x006E1894); }

	void _SetAI()
	{
		if (s_bDisableWOTRAI) SetHuman();
		else SetAI();
	}
};
};

namespace wb2x
{
};

void patch();


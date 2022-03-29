#pragma once

#include <patcher.h>

#include "dllmain.h"

namespace bfme1
{
struct GlobalData;
extern GlobalData *&TheWriteableGlobalData;

extern uint32_t &GameFlags;
};

namespace wb1
{
};

namespace bfme2x
{
struct GlobalData;
extern GlobalData *&TheWriteableGlobalData;

extern uint32_t &GameFlags;

extern bool &g_bEditSystemCreateAHeroes;
};

namespace wb2x
{
};

void patch();


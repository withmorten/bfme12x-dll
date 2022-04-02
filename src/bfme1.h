#pragma once

#define is_bfme1() (*(uint32_t *)0x004600F0 == 0x6AEC8B55)
#define is_wb1() (*(uint32_t *)0x00E2B0AC == 0x0010D9E9)

namespace bfme1
{
void patch();
}

namespace wb1
{
void patch();
}

#pragma once

#define is_bfme2x() (*(uint32_t *)0x004027F7 == 0xB6F7B4B8)
#define is_wb2x() (*(uint32_t *)0x018463E7 == 0x000FF6E9)

#define is_bfme2x_202() (*(uint32_t *)0x006D3172 == 0x90909090)
#define is_bfme2x_202_850() (*(uint32_t *)0x005D8A65 == 0x008F1597)

namespace bfme2x
{
void patch();
}

namespace wb2x
{
void patch();
}

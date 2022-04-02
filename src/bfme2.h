#pragma once

#define is_bfme2() (*(uint32_t *)0x00402C39 == 0xB5B5AAB8)
#define is_wb2() (*(uint32_t *)0x01806837 == 0x001008E9)

namespace bfme2
{
void patch();
}

namespace wb2
{
void patch();
}

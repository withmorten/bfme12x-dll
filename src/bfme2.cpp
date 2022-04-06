#include "patch.h"
#include "bfme2.h"

namespace bfme2
{
struct LivingWorldLogic
{
	void AddPlayer(void *player, bool isLocalPlayer, int isHumanOrNot, void *currGameSlot) { XCALL(0x006BA8F1); }

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

void patch()
{
	if (get_private_profile_bool("must_have", TRUE))
	{
		// disable WinVerifyTrust check, fixes random version
		Nop(0x00637E0A, 6 + 2 + 6 + 2);

		// disable needing the launcher
		Nop(0x004030B0, 5 + 2 + 2 + 5 + 2);
		PatchByte(0x004030C0, 0xEB);

		// fix auto defeat
		BYTE sub_632D38[] = { 0xB0, 0x01, 0xC3 };
		PatchBytes(0x00632D38, sub_632D38);

		// disable langdata.dat loading
		const char *langdata = "censored.dat";
		PatchString(0x00C19734, "censored.dat");
	}

	if (get_private_profile_bool("no_logo", TRUE))
	{
		Patch(0x00636A0A + 2, 0xAF3); // GlobalData::GlobalData()
		Patch(0x00636A11 + 2, 0xAF2); // GlobalData::GlobalData()
	}

	if (get_private_profile_bool("fix_wotr", FALSE))
	{
		// LivingWorldCampaign::CreatePlayers()
		InjectHook(0x0092C65D, &LivingWorldLogic::_AddPlayer);
		InjectHook(0x0092C6F5, &LivingWorldLogic::_AddPlayer);
	}
}
};

namespace wb2
{
void patch()
{
	if (get_private_profile_bool("compression_none_default", TRUE))
	{
		Patch(0x006BFD60 + 1, 0); // CompressionManager::getPreferredCompression()

		Nop(0x0056BB86, 3 + 1 + 5 + 5 + 1 + 3 + 5 + 3 + 4 + 2 + 2); // MapSettings::OnInitDialog()
	}
}
};

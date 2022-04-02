#include "patch.h"
#include "bfme1.h"
#include "bfme2.h"
#include "bfme2x.h"

void patch()
{
	if (is_bfme1())
	{
		bfme1::patch();
	}
	else if (is_bfme2())
	{
		bfme2::patch();
	}
	else if (is_bfme2x())
	{
		bfme2x::patch();
	}
	else if (is_wb1())
	{
		wb1::patch();
	}
	else if (is_wb2())
	{
		wb2::patch();
	}
	else if (is_wb2x())
	{
		wb2x::patch();
	}
}

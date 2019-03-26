#include "stdafx.h"
#include "DebugRob.h"
#include "autocarDlg.h"

using namespace Debug;
using namespace DebugLabComm;

int DebugRob::_receiveTag = 0;

int DebugRob::ReceiveCommand(void)
{
	if(_receiveTag == 1)
	{
		_receiveTag = 0;
		return 1;
	}
	return 0;
}

void DebugRob::UpdateReceiveTag(void)
{
	_receiveTag = 1;
}
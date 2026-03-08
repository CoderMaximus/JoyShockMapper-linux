#pragma once
#include "InputHelpers.h"
#include "JslWrapper.h"


namespace JSM
{

class AutoConnect : public PollingThread
{
public:
	AutoConnect(shared_ptr<JslWrapper> joyshock, bool start);
	virtual ~AutoConnect() = default;

private:
	bool AutoConnectPoll(void* param);
	shared_ptr<JslWrapper> jsl;
	int lastSize = 0;
};

} //JSM
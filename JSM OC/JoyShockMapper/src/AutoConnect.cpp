#include "AutoConnect.h"
#include "JslWrapper.h"
#include "InputHelpers.h"
#include "Gamepad.h"


namespace JSM
{

AutoConnect::AutoConnect(shared_ptr<JslWrapper> joyshock, bool start)
  : PollingThread("AutoConnect thread", std::bind(&AutoConnect::AutoConnectPoll, this, std::placeholders::_1), nullptr, 1000, start)
  , jsl(joyshock)
{
}

bool AutoConnect::AutoConnectPoll(void* param)
{
	int realSize = jsl->GetDeviceCount() - Gamepad::getCount();
	if(lastSize != realSize)
	{
		COUT_INFO << "[AUTOCONNECT] Going from " << lastSize << " devices to " << realSize << ".\n";
		lastSize = realSize;
		WriteToConsole("RECONNECT_CONTROLLERS");
	}
	return true;
}

} // namespace JSM

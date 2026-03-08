#include "Gamepad.h"
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <linux/input.h>
#include <fcntl.h>
#include <cstring>
#include <map>

size_t Gamepad::_count = 0;

Gamepad::Gamepad()
{
	++_count;
	DEBUG_LOG << "[DEBUG] Gamepad constructor: _count is now " << _count << endl;
}

Gamepad::~Gamepad()
{
	--_count;
	DEBUG_LOG << "[DEBUG] Gamepad destructor: _count is now " << _count << endl;
}


class GamepadImpl : public Gamepad
{
private:
	libevdev* device_ = nullptr;
	libevdev_uinput* uinput_device_ = nullptr;
	ControllerScheme scheme_;
	bool initialized_ = false;
	std::string error_msg_;
	
	// Map JSM button codes to Linux input event codes
	std::map<uint16_t, int> buttonMap_;
	
	// D-pad state tracking for combined presses
	bool dpad_up_ = false;
	bool dpad_down_ = false;
	bool dpad_left_ = false;
	bool dpad_right_ = false;
	
	void updateDpadX()
	{
		int value = 0;
		if (dpad_left_ && !dpad_right_) value = -1;
		else if (dpad_right_ && !dpad_left_) value = 1;
		libevdev_uinput_write_event(uinput_device_, EV_ABS, ABS_HAT0X, value);
	}
	
	void updateDpadY()
	{
		int value = 0;
		if (dpad_up_ && !dpad_down_) value = -1;
		else if (dpad_down_ && !dpad_up_) value = 1;
		libevdev_uinput_write_event(uinput_device_, EV_ABS, ABS_HAT0Y, value);
	}
	
	void initButtonMap()
	{
		// Xbox/PS button mappings to Linux gamepad buttons
		buttonMap_[X_A] = BTN_SOUTH;      // A button
		buttonMap_[X_B] = BTN_EAST;       // B button
		buttonMap_[X_X] = BTN_NORTH;      // X button
		buttonMap_[X_Y] = BTN_WEST;       // Y button
		buttonMap_[X_LB] = BTN_TL;        // Left bumper
		buttonMap_[X_RB] = BTN_TR;        // Right bumper
		buttonMap_[X_BACK] = BTN_SELECT;  // Back/Select
		buttonMap_[X_START] = BTN_START;  // Start
		buttonMap_[X_GUIDE] = BTN_MODE;   // Guide/Home
		buttonMap_[X_LS] = BTN_THUMBL;    // Left stick click
		buttonMap_[X_RS] = BTN_THUMBR;    // Right stick click
		
		// PS buttons (same mappings)
		buttonMap_[PS_CROSS] = BTN_SOUTH;
		buttonMap_[PS_CIRCLE] = BTN_EAST;
		buttonMap_[PS_SQUARE] = BTN_NORTH;
		buttonMap_[PS_TRIANGLE] = BTN_WEST;
		buttonMap_[PS_L1] = BTN_TL;
		buttonMap_[PS_R1] = BTN_TR;
		buttonMap_[PS_SHARE] = BTN_SELECT;
		buttonMap_[PS_OPTIONS] = BTN_START;
		buttonMap_[PS_HOME] = BTN_MODE;
		buttonMap_[PS_L3] = BTN_THUMBL;
		buttonMap_[PS_R3] = BTN_THUMBR;
	}

public:
	GamepadImpl(ControllerScheme scheme)
	  : scheme_(scheme)
	{
		initButtonMap();
		
		device_ = libevdev_new();
		if (!device_)
		{
			error_msg_ = "Failed to create libevdev device";
			return;
		}
		
		// Set device name based on scheme
		if (scheme == ControllerScheme::XBOX)
		{
			libevdev_set_name(device_, "JSM Virtual Xbox Controller");
			libevdev_set_id_vendor(device_, 0x1209);  // pid.codes vendor ID (for open source projects)
			libevdev_set_id_product(device_, 0x4a53); // "JS" in hex (JoyShockMapper)
		}
		else // DS4
		{
			libevdev_set_name(device_, "JSM Virtual DualShock 4");
			libevdev_set_id_vendor(device_, 0x1209);  // pid.codes vendor ID (for open source projects)
			libevdev_set_id_product(device_, 0x4a54); // "JT" in hex (JoyShockMapper Touchpad)
		}
		libevdev_set_id_bustype(device_, BUS_USB);
		libevdev_set_id_version(device_, 0x0111);
		
		// Enable button events
		libevdev_enable_event_type(device_, EV_KEY);
		libevdev_enable_event_code(device_, EV_KEY, BTN_SOUTH, nullptr);    // A/Cross
		libevdev_enable_event_code(device_, EV_KEY, BTN_EAST, nullptr);     // B/Circle
		libevdev_enable_event_code(device_, EV_KEY, BTN_NORTH, nullptr);    // X/Square
		libevdev_enable_event_code(device_, EV_KEY, BTN_WEST, nullptr);     // Y/Triangle
		libevdev_enable_event_code(device_, EV_KEY, BTN_TL, nullptr);       // LB/L1
		libevdev_enable_event_code(device_, EV_KEY, BTN_TR, nullptr);       // RB/R1
		libevdev_enable_event_code(device_, EV_KEY, BTN_SELECT, nullptr);   // Back/Share
		libevdev_enable_event_code(device_, EV_KEY, BTN_START, nullptr);    // Start/Options
		libevdev_enable_event_code(device_, EV_KEY, BTN_MODE, nullptr);     // Guide/Home
		libevdev_enable_event_code(device_, EV_KEY, BTN_THUMBL, nullptr);   // L3
		libevdev_enable_event_code(device_, EV_KEY, BTN_THUMBR, nullptr);   // R3
		libevdev_enable_event_code(device_, EV_KEY, BTN_TL2, nullptr);      // LT (digital)
		libevdev_enable_event_code(device_, EV_KEY, BTN_TR2, nullptr);      // RT (digital)
		
		// Enable absolute axes for sticks and triggers
		libevdev_enable_event_type(device_, EV_ABS);
		
		// Left stick (X, Y)
		struct input_absinfo absinfo;
		memset(&absinfo, 0, sizeof(absinfo));
		absinfo.minimum = -32768;
		absinfo.maximum = 32767;
		absinfo.fuzz = 255;
		absinfo.flat = 4095;
		libevdev_enable_event_code(device_, EV_ABS, ABS_X, &absinfo);
		libevdev_enable_event_code(device_, EV_ABS, ABS_Y, &absinfo);
		
		// Right stick (RX, RY)
		libevdev_enable_event_code(device_, EV_ABS, ABS_RX, &absinfo);
		libevdev_enable_event_code(device_, EV_ABS, ABS_RY, &absinfo);
		
		// Triggers (Z, RZ) - analog
		struct input_absinfo triggerinfo;
		memset(&triggerinfo, 0, sizeof(triggerinfo));
		triggerinfo.minimum = 0;
		triggerinfo.maximum = 255;
		libevdev_enable_event_code(device_, EV_ABS, ABS_Z, &triggerinfo);   // Left trigger
		libevdev_enable_event_code(device_, EV_ABS, ABS_RZ, &triggerinfo);  // Right trigger
		
		// D-pad (HAT0X, HAT0Y)
		struct input_absinfo hatinfo;
		memset(&hatinfo, 0, sizeof(hatinfo));
		hatinfo.minimum = -1;
		hatinfo.maximum = 1;
		libevdev_enable_event_code(device_, EV_ABS, ABS_HAT0X, &hatinfo);
		libevdev_enable_event_code(device_, EV_ABS, ABS_HAT0Y, &hatinfo);
		
		// Create the uinput device
		int error = libevdev_uinput_create_from_device(device_,
		                                               LIBEVDEV_UINPUT_OPEN_MANAGED,
		                                               &uinput_device_);
		
		if (error != 0)
		{
			error_msg_ = "Failed to create uinput device: ";
			error_msg_ += strerror(-error);
			libevdev_free(device_);
			device_ = nullptr;
			return;
		}
		
		initialized_ = true;
	}

	virtual ~GamepadImpl()
	{
		if (uinput_device_)
		{
			libevdev_uinput_destroy(uinput_device_);
		}
		if (device_)
		{
			libevdev_free(device_);
		}
	}

	virtual bool isInitialized(std::string* errorMsg = nullptr) const override
	{
		if (errorMsg && !initialized_)
		{
			*errorMsg = error_msg_;
		}
		return initialized_;
	}

	virtual void setButton(KeyCode btn, bool pressed) override
	{
		if (!initialized_) return;
		
		auto it = buttonMap_.find(btn.code);
		if (it != buttonMap_.end())
		{
			libevdev_uinput_write_event(uinput_device_, EV_KEY, it->second, pressed ? 1 : 0);
		}
		// Handle D-pad separately - track state for combined presses
		else if (btn.code == X_UP || btn.code == PS_UP)
		{
			dpad_up_ = pressed;
			updateDpadY();
		}
		else if (btn.code == X_DOWN || btn.code == PS_DOWN)
		{
			dpad_down_ = pressed;
			updateDpadY();
		}
		else if (btn.code == X_LEFT || btn.code == PS_LEFT)
		{
			dpad_left_ = pressed;
			updateDpadX();
		}
		else if (btn.code == X_RIGHT || btn.code == PS_RIGHT)
		{
			dpad_right_ = pressed;
			updateDpadX();
		}
		// Handle digital trigger presses
		else if (btn.code == X_LT || btn.code == PS_L2)
		{
			setLeftTrigger(pressed ? 1.0f : 0.0f);
		}
		else if (btn.code == X_RT || btn.code == PS_R2)
		{
			setRightTrigger(pressed ? 1.0f : 0.0f);
		}
	}

	virtual void setLeftStick(float x, float y) override
	{
		if (!initialized_) return;
		
		// Convert -1.0 to 1.0 range to -32768 to 32767
		int xVal = static_cast<int>(x * 32767.0f);
		int yVal = static_cast<int>(y * 32767.0f);
		
		libevdev_uinput_write_event(uinput_device_, EV_ABS, ABS_X, xVal);
		libevdev_uinput_write_event(uinput_device_, EV_ABS, ABS_Y, yVal);
	}

	virtual void setRightStick(float x, float y) override
	{
		if (!initialized_) return;
		
		// Convert -1.0 to 1.0 range to -32768 to 32767
		int xVal = static_cast<int>(x * 32767.0f);
		int yVal = static_cast<int>(y * 32767.0f);
		
		libevdev_uinput_write_event(uinput_device_, EV_ABS, ABS_RX, xVal);
		libevdev_uinput_write_event(uinput_device_, EV_ABS, ABS_RY, yVal);
	}
	
	virtual void setStick(float x, float y, bool isLeft) override
	{
		if (isLeft)
		{
			setLeftStick(x, y);
		}
		else
		{
			setRightStick(x, y);
		}
	}

	virtual void setLeftTrigger(float value) override
	{
		if (!initialized_) return;
		
		// Convert 0.0 to 1.0 range to 0 to 255
		int val = static_cast<int>(value * 255.0f);
		libevdev_uinput_write_event(uinput_device_, EV_ABS, ABS_Z, val);
		
		// Also set digital trigger button if pressed enough
		libevdev_uinput_write_event(uinput_device_, EV_KEY, BTN_TL2, value > 0.5f ? 1 : 0);
	}

	virtual void setRightTrigger(float value) override
	{
		if (!initialized_) return;
		
		// Convert 0.0 to 1.0 range to 0 to 255
		int val = static_cast<int>(value * 255.0f);
		libevdev_uinput_write_event(uinput_device_, EV_ABS, ABS_RZ, val);
		
		// Also set digital trigger button if pressed enough
		libevdev_uinput_write_event(uinput_device_, EV_KEY, BTN_TR2, value > 0.5f ? 1 : 0);
	}

	virtual void update() override
	{
		if (!initialized_) return;
		
		// Send SYN_REPORT to commit all changes
		libevdev_uinput_write_event(uinput_device_, EV_SYN, SYN_REPORT, 0);
	}
	
	virtual ControllerScheme getType() const override
	{
		return scheme_;
	}
	
	virtual void setGyro(TimePoint now, float accelX, float accelY, float accelZ, float gyroX, float gyroY, float gyroZ) override
	{
		// Not implemented for basic gamepad - could be added for DS4 motion support
	}
	
	virtual void setTouchState(optional<FloatXY> press1, optional<FloatXY> press2) override
	{
		// Not implemented for basic gamepad - could be added for DS4 touchpad support
	}
};

Gamepad *Gamepad::getNew(ControllerScheme scheme, Callback notification)
{
	return new GamepadImpl(scheme);
}

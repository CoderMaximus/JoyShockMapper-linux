#include <cstdlib>

#include <string>
#include <cstring>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "JoyShockMapper.h"
#include "PlatformDefinitions.h"


// https://www.theurbanpenguin.com/4184-2/
#define FOREGROUND_BLUE 34 // text color is blue.
#define FOREGROUND_GREEN 32 // text color is green.
#define FOREGROUND_RED 31 // text color is red.
#define FOREGROUND_YELLOW 33 // Text color is yellow
#define FOREGROUND_INTENSITY 0x0100 // text color is bold.
#define DEFAULT_COLOR 37 // text color is white

template<std::ostream *stdio, uint16_t color>
struct ColorStream : public std::stringbuf
{
	~ColorStream()
	{

		(*stdio) << "\033[" << (color >> 8) << ';' << (color & 0x00FF) << 'm' << str() << "\033[0;" << DEFAULT_COLOR << 'm';
	}
};

streambuf *Log::makeBuffer(Level level)
{
	switch (level)
	{
	case Level::ERR:
		return new ColorStream<&std::cerr, FOREGROUND_RED | FOREGROUND_INTENSITY>();
	case Level::WARN:
		return new ColorStream<&cout, FOREGROUND_YELLOW | FOREGROUND_INTENSITY>();
	case Level::INFO:
		return new ColorStream<&cout, FOREGROUND_BLUE | FOREGROUND_INTENSITY>();
#if defined(NDEBUG) // release
	case Level::UT:
		return new NullBuffer(); // unused
#else
	case Level::UT:
		return new ColorStream<&cout, FOREGROUND_BLUE | FOREGROUND_RED>(); // purplish
#endif
	case Level::BOLD:
		return new ColorStream<&cout, FOREGROUND_GREEN | FOREGROUND_INTENSITY>();
	default:
		return new ColorStream<&std::cout, FOREGROUND_GREEN>();
	}
}

const char *AUTOLOAD_FOLDER() {
	std::string directory;

	const auto XDG_CONFIG_HOME = getenv("XDG_CONFIG_HOME");
	if (XDG_CONFIG_HOME == nullptr)
	{
		directory = std::string{ getenv("HOME") } + "/.config";
	}
	else
	{
		directory = XDG_CONFIG_HOME;
	}

	directory = directory + "/JoyShockMapper/AutoLoad/";
	return strdup(directory.c_str());
};

const char *GYRO_CONFIGS_FOLDER() {
	std::string directory;

	const auto XDG_CONFIG_HOME = getenv("XDG_CONFIG_HOME");
	if (XDG_CONFIG_HOME == nullptr)
	{
		directory = std::string{ getenv("HOME") } + "/.config";
	}
	else
	{
		directory = XDG_CONFIG_HOME;
	}

	directory = directory + "/JoyShockMapper/GyroConfigs/";
	return strdup(directory.c_str());
};

const char *BASE_JSM_CONFIG_FOLDER() {
	std::string directory;

	const auto XDG_CONFIG_HOME = getenv("XDG_CONFIG_HOME");
	if (XDG_CONFIG_HOME == nullptr)
	{
		directory = std::string{ getenv("HOME") } + "/.config";
	}
	else
	{
		directory = XDG_CONFIG_HOME;
	}

	directory = directory + "/JoyShockMapper/";
	return strdup(directory.c_str());
};

unsigned long GetCurrentProcessId()
{
	return ::getpid();
}

/// Valid inputs:
/// 0-9, N0-N9, F1-F29, A-Z, (L, R, )CONTROL, (L, R, )ALT, (L, R, )SHIFT, TAB, ENTER
/// (L, M, R)MOUSE, SCROLL(UP, DOWN)
/// NONE
/// And characters: ; ' , . / \ [ ] + - `
/// Yes, this looks slow. But it's only there to help set up faster mappings
WORD nameToKey(string_view name)
{
	// https://msdn.microsoft.com/en-us/library/dd375731%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
	auto length = name.length();

	// Handle KEY_ prefix for explicit keyboard keys
	if (length > 4 && name.substr(0, 4) == "KEY_")
	{
		string_view keyName = name.substr(4);
		// Try single letter keys first
		if (keyName.length() == 1)
		{
			char c = keyName[0];
			if (c >= 'A' && c <= 'Z')
				return c - 'A' + 0x41;
			if (c >= '0' && c <= '9')
				return c - '0' + 0x30;
		}
		// Function keys
		if (keyName.length() == 2 && keyName[0] == 'F')
		{
			if (keyName[1] >= '1' && keyName[1] <= '9')
				return 0x70 - 1 + (keyName[1] - '0'); // VK_F1
			if (keyName == "F10") return 0x79;
			if (keyName == "F11") return 0x7A;
			if (keyName == "F12") return 0x7B;
		}
		if (keyName == "ESC") return 0x1B;
		if (keyName == "TAB") return 0x09;
		if (keyName == "CAPSLOCK") return 0x14;
		if (keyName == "LSHIFT") return 0xA0;
		if (keyName == "RSHIFT") return 0xA1;
		if (keyName == "LCTRL") return 0xA2;
		if (keyName == "RCTRL") return 0xA3;
		if (keyName == "LALT") return 0xA4;
		if (keyName == "RALT") return 0xA5;
		if (keyName == "SPACE") return 0x20;
		if (keyName == "ENTER") return 0x0D;
		if (keyName == "BACKSPACE") return 0x08;
		if (keyName == "INSERT") return 0x2D;
		if (keyName == "DELETE") return 0x2E;
		if (keyName == "HOME") return 0x24;
		if (keyName == "END") return 0x23;
		if (keyName == "PAGEUP") return 0x21;
		if (keyName == "PAGEDOWN") return 0x22;
		if (keyName == "UP") return 0x26;
		if (keyName == "DOWN") return 0x28;
		if (keyName == "LEFT") return 0x25;
		if (keyName == "RIGHT") return 0x27;
		if (keyName == "TILDE") return 0xC0;
		if (keyName == "MINUS") return 0xBD;
		if (keyName == "EQUALS") return 0xBB;
		if (keyName == "LEFTBRACKET") return 0xDB;
		if (keyName == "RIGHTBRACKET") return 0xDD;
		if (keyName == "BACKSLASH") return 0xDC;
		if (keyName == "SEMICOLON") return 0xBA;
		if (keyName == "QUOTE") return 0xDE;
		if (keyName == "COMMA") return 0xBC;
		if (keyName == "PERIOD") return 0xBE;
		if (keyName == "SLASH") return 0xBF;
	}
	if (length == 1)
	{
		// direct mapping to a number or character key
		char character = name.at(0);
		if (character >= '0' && character <= '9')
		{
			return character - '0' + 0x30;
		}
		if (character >= 'A' && character <= 'Z')
		{
			return character - 'A' + 0x41;
		}
		if (character == '+')
		{
			return VK_OEM_PLUS;
		}
		if (character == '-')
		{
			return VK_OEM_MINUS;
		}
		if (character == ',')
		{
			return VK_OEM_COMMA;
		}
		if (character == '.')
		{
			return VK_OEM_PERIOD;
		}
		if (character == ';')
		{
			return VK_OEM_1;
		}
		if (character == '/')
		{
			return VK_OEM_2;
		}
		if (character == '`')
		{
			return VK_OEM_3;
		}
		if (character == '[')
		{
			return VK_OEM_4;
		}
		if (character == '\\')
		{
			return VK_OEM_5;
		}
		if (character == ']')
		{
			return VK_OEM_6;
		}
		if (character == '\'')
		{
			return VK_OEM_7;
		}
	}
	if (length == 2)
	{
		// function key?
		char character = name.at(0);
		char character2 = name.at(1);
		if (character == 'F')
		{
			if (character2 >= '1' && character2 <= '9')
			{
				return character2 - '1' + VK_F1;
			}
		}
		else if (character == 'N')
		{
			if (character2 >= '0' && character2 <= '9')
			{
				return character2 - '0' + VK_NUMPAD0;
			}
		}
	}
	if (length == 3)
	{
		// could be function keys still
		char character = name.at(0);
		char character2 = name.at(1);
		char character3 = name.at(2);
		if (character == 'F')
		{
			if (character2 == '1' || character2 <= '2')
			{
				if (character3 >= '0' && character3 <= '9')
				{
					return (character2 - '1') * 10 + VK_F10 + (character3 - '0');
				}
			}
		}
	}
	if (length > 2 && name[0] == '"' && name[length - 1] == '"')
    {
        return COMMAND_ACTION;
    }
	if (name.compare("LEFT") == 0)
	{
		return VK_LEFT;
	}
	if (name.compare("RIGHT") == 0)
	{
		return VK_RIGHT;
	}
	if (name.compare("UP") == 0)
	{
		return VK_UP;
	}
	if (name.compare("DOWN") == 0)
	{
		return VK_DOWN;
	}
	if (name.compare("SPACE") == 0)
	{
		return VK_SPACE;
	}
	if (name.compare("CONTROL") == 0)
	{
		return VK_CONTROL;
	}
	if (name.compare("LCONTROL") == 0)
	{
		return VK_LCONTROL;
	}
	if (name.compare("RCONTROL") == 0)
	{
		return VK_RCONTROL;
	}
	if (name.compare("SHIFT") == 0)
	{
		return VK_SHIFT;
	}
	if (name.compare("LSHIFT") == 0)
	{
		return VK_LSHIFT;
	}
	if (name.compare("RSHIFT") == 0)
	{
		return VK_RSHIFT;
	}
	if (name.compare("ALT") == 0)
	{
		return VK_MENU;
	}
	if (name.compare("LALT") == 0)
	{
		return VK_LMENU;
	}
	if (name.compare("RALT") == 0)
	{
		return VK_RMENU;
	}
	if (name.compare("TAB") == 0)
	{
		return VK_TAB;
	}
	if (name.compare("ENTER") == 0)
	{
		return VK_RETURN;
	}
	if (name.compare("ESC") == 0)
	{
		return VK_ESCAPE;
	}
	if (name.compare("PAGEUP") == 0)
	{
		return VK_PRIOR;
	}
	if (name.compare("PAGEDOWN") == 0)
	{
		return VK_NEXT;
	}
	if (name.compare("HOME") == 0)
	{
		return VK_HOME;
	}
	if (name.compare("END") == 0)
	{
		return VK_END;
	}
	if (name.compare("INSERT") == 0)
	{
		return VK_INSERT;
	}
	if (name.compare("DELETE") == 0)
	{
		return VK_DELETE;
	}
	if (name.compare("LMOUSE") == 0)
	{
		return VK_LBUTTON;
	}
	if (name.compare("RMOUSE") == 0)
	{
		return VK_RBUTTON;
	}
	if (name.compare("MMOUSE") == 0)
	{
		return VK_MBUTTON;
	}
	if (name.compare("BMOUSE") == 0)
	{
		return VK_XBUTTON1;
	}
	if (name.compare("FMOUSE") == 0)
	{
		return VK_XBUTTON2;
	}
	if (name.compare("SCROLLDOWN") == 0)
	{
		return V_WHEEL_DOWN;
	}
	if (name.compare("SCROLLUP") == 0)
	{
		return V_WHEEL_UP;
	}
	if (name.compare("BACKSPACE") == 0)
	{
		return VK_BACK;
	}
	if (name.compare("LWINDOWS") == 0)
	{
		return VK_LWIN;
	}
	if (name.compare("RWINDOWS") == 0)
	{
		return VK_RWIN;
	}
	if (name.compare("CONTEXT") == 0)
	{
		return VK_APPS;
	}
	if (name.compare("SCREENSHOT") == 0)
	{
		return VK_SNAPSHOT;
	}
	if (name.compare("NONE") == 0)
	{
		return NO_HOLD_MAPPED;
	}
	if (name.compare("CALIBRATE") == 0)
	{
		return CALIBRATE;
	}
	if (name.compare("GYRO_INV_X") == 0)
	{
		return GYRO_INV_X;
	}
	if (name.compare("GYRO_INV_Y") == 0)
	{
		return GYRO_INV_Y;
	}
	if (name.compare("GYRO_INVERT") == 0)
	{
		return GYRO_INVERT;
	}
	if (name.compare("GYRO_TRACK_X") == 0)
	{
		return GYRO_TRACK_X;
	}
	if (name.compare("GYRO_TRACK_Y") == 0)
	{
		return GYRO_TRACK_Y;
	}
	if (name.compare("GYRO_TRACKBALL") == 0)
	{
		return GYRO_TRACKBALL;
	}
	if (name.compare("GYRO_ON") == 0)
	{
		return GYRO_ON_BIND;
	}
	if (name.compare("GYRO_OFF") == 0)
	{
		return GYRO_OFF_BIND;
	}
	// Xbox virtual controller buttons
	if (name.compare("X_UP") == 0)
	{
		return X_UP;
	}
	if (name.compare("X_DOWN") == 0)
	{
		return X_DOWN;
	}
	if (name.compare("X_LEFT") == 0)
	{
		return X_LEFT;
	}
	if (name.compare("X_RIGHT") == 0)
	{
		return X_RIGHT;
	}
	if (name.compare("X_LB") == 0)
	{
		return X_LB;
	}
	if (name.compare("X_RB") == 0)
	{
		return X_RB;
	}
	if (name.compare("X_X") == 0)
	{
		return X_X;
	}
	if (name.compare("X_A") == 0)
	{
		return X_A;
	}
	if (name.compare("X_Y") == 0)
	{
		return X_Y;
	}
	if (name.compare("X_B") == 0)
	{
		return X_B;
	}
	if (name.compare("X_LS") == 0)
	{
		return X_LS;
	}
	if (name.compare("X_RS") == 0)
	{
		return X_RS;
	}
	if (name.compare("X_BACK") == 0)
	{
		return X_BACK;
	}
	if (name.compare("X_START") == 0)
	{
		return X_START;
	}
	if (name.compare("X_GUIDE") == 0)
	{
		return X_GUIDE;
	}
	if (name.compare("X_LT") == 0)
	{
		return X_LT;
	}
	if (name.compare("X_RT") == 0)
	{
		return X_RT;
	}
	// PlayStation virtual controller buttons (aliases)
	if (name.compare("PS_UP") == 0)
	{
		return PS_UP;
	}
	if (name.compare("PS_DOWN") == 0)
	{
		return PS_DOWN;
	}
	if (name.compare("PS_LEFT") == 0)
	{
		return PS_LEFT;
	}
	if (name.compare("PS_RIGHT") == 0)
	{
		return PS_RIGHT;
	}
	if (name.compare("PS_L1") == 0)
	{
		return PS_L1;
	}
	if (name.compare("PS_R1") == 0)
	{
		return PS_R1;
	}
	if (name.compare("PS_SQUARE") == 0)
	{
		return PS_SQUARE;
	}
	if (name.compare("PS_CROSS") == 0)
	{
		return PS_CROSS;
	}
	if (name.compare("PS_TRIANGLE") == 0)
	{
		return PS_TRIANGLE;
	}
	if (name.compare("PS_CIRCLE") == 0)
	{
		return PS_CIRCLE;
	}
	if (name.compare("PS_L3") == 0)
	{
		return PS_L3;
	}
	if (name.compare("PS_R3") == 0)
	{
		return PS_R3;
	}
	if (name.compare("PS_SHARE") == 0)
	{
		return PS_SHARE;
	}
	if (name.compare("PS_OPTIONS") == 0)
	{
		return PS_OPTIONS;
	}
	if (name.compare("PS_HOME") == 0)
	{
		return PS_HOME;
	}
	if (name.compare("PS_PAD_CLICK") == 0)
	{
		return PS_PAD_CLICK;
	}
	if (name.compare("PS_L2") == 0)
	{
		return PS_L2;
	}
	if (name.compare("PS_R2") == 0)
	{
		return PS_R2;
	}
	return 0x00;
}

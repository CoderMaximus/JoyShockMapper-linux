# JoyShockMapper (Linux Optimized Fork)

This is a fork of Electronicks/JoyShockMapper specifically tuned for Linux. While the original version may fail to build on modern Linux distributions, this fork introduces necessary build fixes and expanded functionality.
## Key Features

• **Virtual Xbox Controller**: Native support for creating a virtual Xbox controller that is recognized system-wide (visible in /proc/bus/input/devices).

• **Keyboard Mapping**: Capability to map standard keyboard inputs directly to controller buttons.

• **Linux Build Fixes**: Resolved dependencies and compilation errors specific to the Linux environment.

# Important Notes

• **Hardware Compatibility**: This fork was primarily developed and tested using a 3rd-party Switch Pro Controller. While it should theoretically support DualSense, DS4, and JoyCons like the original JSM, you might experience some hiccups(although highly unlikely), if you encounter any issue, please open a github issue regarding it.

• **Controller "Clashing"**: Since Linux does not have a direct equivalent to Windows' HidHide, games may see both your physical controller and the virtual Xbox controller simultaneously.

## How to hide your physical controller:

If you experience double-input issues, you can force games to ignore the physical device using environment variables.

### For Heroic Games Launcher / Steam / Lutris:
Add the following environment variable to your game or launcher settings:

    Variable: SDL_GAMECONTROLLER_IGNORE_DEVICES

    Value: 0xVID/0xPID
    (Replace with your controller's specific Vendor ID and Product ID)

#### Note:
The fix above should work with any SDL controller like Pro Controllers, DualShock, DualSense, etc...

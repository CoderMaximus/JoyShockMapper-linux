# Virtual Xbox Controller with Gyro Mouse Setup

## What This Does

This configuration allows JoyShockMapper to:
1. **Create a virtual Xbox controller** that appears in `/proc/bus/input/devices` - this is what Heroic Launcher will see
2. **Map gyro to mouse** for precise aiming
3. **Map all Pro Controller buttons and sticks** to the virtual Xbox controller for normal game input

## Prerequisites

### Linux Requirements
You need read/write access to `/dev/uinput`. This can be done by:

1. **Apply udev rules** (recommended):
   ```bash
   sudo ./install_udev_rules.sh
   sudo usermod -a -G input $USER
   # Reboot for changes to take effect
   sudo reboot
   ```

2. **Or manually set permissions** (temporary):
   ```bash
   sudo chmod 666 /dev/uinput
   ```

## How to Use

1. **Start JoyShockMapper**:
   ```bash
   ./JoyShockMapper
   ```

2. **Load the configuration**:
   ```
   gyro_with_virtual_xbox.txt
   ```

3. **Verify the virtual controller**:
   Open a new terminal and check:
   ```bash
   cat /proc/bus/input/devices | grep -A 20 "JSM Virtual Xbox"
   ```
   
   You should see something like:
   ```
   N: Name="JSM Virtual Xbox Controller"
   I: Bus=0003 Vendor=045e Product=028e Version=0111
   ```

4. **Test with evtest** (optional):
   ```bash
   sudo evtest
   # Select the "JSM Virtual Xbox Controller" device
   # Press buttons and move sticks to verify it works
   ```

## Configuration Details

### Gyro Behavior
- **Gyro → Mouse**: For aiming in games
- **Gyro disable button**: CAPTURE (touchpad click on Pro Controller)
- **Sensitivity**: Adaptive from 1 to 2 based on movement speed
- **Threshold**: 0-75 (very responsive)

### Stick Mapping
- **Left Stick → Virtual Xbox Left Stick**: For movement
- **Right Stick → Virtual Xbox Right Stick**: For camera/secondary input
- **Sensitivity**: 180 with power curve of 3
- **Acceleration**: 1x rate, 2x cap

### Button Mapping
All Pro Controller buttons are mapped to their Xbox equivalents:
- S → A (South)
- E → B (East)  
- W → X (West)
- N → Y (North)
- L → LB, R → RB
- ZL → LT, ZR → RT
- D-pad → D-pad
- MINUS → Back, PLUS → Start
- HOME → Guide
- L3/R3 → LS/RS (stick clicks)

## Customization

### Change Gyro Sensitivity
```
MIN_GYRO_SENS = 0.5  # Lower = less sensitive
MAX_GYRO_SENS = 3    # Higher = more sensitive at high speeds
```

### Use Flick Stick (optional)
Uncomment this line for flick stick on right stick:
```
RIGHT_STICK_MODE = FLICK
```

### Different Gyro Disable Button
```
GYRO_OFF = L  # Use L button instead
GYRO_OFF = NONE  # Always on
```

## Troubleshooting

### Virtual controller not appearing
1. Check permissions: `ls -la /dev/uinput`
2. Check user is in input group: `groups $USER`
3. Try running with sudo (not recommended for normal use)

### Heroic still seeing SDL device
In Heroic Launcher settings, you should have something like:
```
SDL_GAMECONTROLLER_IGNORE_DEVICES=0x057e/0x2009
```
This tells SDL to ignore Nintendo Pro Controllers (vendor 0x057e, product 0x2009).

### Gyro not working
- Make sure CAPTURE button isn't pressed (it disables gyro)
- Check gyro calibration: `AUTO_CALIBRATE_GYRO = ON`

### Game sees both controllers
This is normal. The game will use the virtual Xbox controller for input, while gyro provides mouse input for aiming.

## Advanced: Switching Profiles

You can create different configs for different games:

**For games that need gyro always on:**
```
GYRO_OFF = NONE
```

**For games that use right stick for camera:**
```
RIGHT_STICK_MODE = NO_MOUSE  # Don't interfere with virtual controller
```

**For racing games:**
```
GYRO_OUTPUT = LEFT_STICK  # Use gyro for steering
LEFT_STICK_MODE = NO_MOUSE  # Don't use left stick
```

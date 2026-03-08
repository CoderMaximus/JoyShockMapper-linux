# JoyShockMapper Build Notes

## Build Status: ✅ SUCCESS

Successfully built JoyShockMapper with **clang++ 21.1.8** on Linux.

## Build Summary

### Compiler Used
- **Compiler**: clang++ (LLVM) 21.1.8
- **C++ Standard**: C++23
- **Build System**: CMake 3.28+
- **Backend**: SDL3 (built from source)

### Build Command
```bash
mkdir -p build
cd build
cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang ..
make -j$(nproc)
```

### Output Binary
- **Location**: `build/JoyShockMapper/JoyShockMapper`
- **Type**: ELF 64-bit LSB pie executable, x86-64
- **Size**: ~5.4 MB (not stripped)
- **Dependencies**: Links against SDL3 (included in build)

## Fixes Applied

The following issues were fixed to make the code compile with clang++:

### 1. Missing `override` keywords in JSMVariable.hpp
**File**: `JoyShockMapper/include/JSMVariable.hpp`

Added `override` keyword to virtual functions in `ChordedVariable` class (lines 208, 213):
```cpp
virtual operator T() const override  // Line 208
virtual const T &value() const override  // Line 213
```

### 2. Missing `<chrono>` include in Gamepad.h
**File**: `JoyShockMapper/include/Gamepad.h`

Added missing include for std::chrono types:
```cpp
#include <chrono>
```

### 3. Missing `<algorithm>` include in TriggerEffectGenerator.cpp
**File**: `JoyShockMapper/src/TriggerEffectGenerator.cpp`

Added missing include for std::find_if:
```cpp
#include <algorithm>
```

## udev Rules Installation

JoyShockMapper requires proper permissions to access controller devices on Linux.

### Automatic Installation (Recommended)
Run the provided installation script:
```bash
sudo bash install_udev_rules.sh
```

This script will:
1. Install udev rules to `/etc/udev/rules.d/50-joyshockmapper.rules`
2. Add your user to the `input` group
3. Reload udev rules

**Important**: Log out and log back in after installation for group changes to take effect.

### Manual Installation
If you prefer to install manually:

1. Copy the udev rules:
```bash
sudo cp dist/linux/50-joyshockmapper.rules /etc/udev/rules.d/
sudo chmod 644 /etc/udev/rules.d/50-joyshockmapper.rules
```

2. Add your user to the input group:
```bash
sudo usermod -a -G input $USER
```

3. Reload udev rules:
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

4. Log out and log back in.

## Supported Controllers

The udev rules provide access to:
- **DualShock 4** (USB and Bluetooth)
- **DualShock 4 Slim** (USB and Bluetooth)
- **Nintendo Switch Pro Controller** (USB and Bluetooth)
- **uinput** device (for virtual controller creation)

## Running JoyShockMapper

After installing udev rules and logging back in:

```bash
./build/JoyShockMapper/JoyShockMapper
```

## Build Warnings

The build produces some warnings (non-critical):
- `-Wswitch`: Some enum values not handled in switch statements
- `-Winconsistent-missing-override`: Some override keywords missing in macro-generated code
- `-Wstring-compare`: String literal comparisons (in DigitalButton.cpp)

These warnings do not affect functionality but could be addressed in future improvements.

## Testing

To verify the build:
1. Install udev rules (see above)
2. Log out and log back in
3. Connect a supported controller
4. Run `./build/JoyShockMapper/JoyShockMapper`

## Notes

- The build uses SDL3 instead of JoyShockLibrary (JSL) by default
- SDL3 is fetched and built automatically during CMake configuration
- The binary is dynamically linked against the locally built SDL3 library

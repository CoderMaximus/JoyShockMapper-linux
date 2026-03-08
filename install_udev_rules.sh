#!/bin/bash
# JoyShockMapper udev rules installer
# This script installs udev rules to allow non-root access to controllers

set -e

echo "JoyShockMapper udev rules installer"
echo "===================================="
echo ""

RULES_FILE="dist/linux/50-joyshockmapper.rules"
DEST_DIR="/etc/udev/rules.d"
DEST_FILE="$DEST_DIR/50-joyshockmapper.rules"

if [ ! -f "$RULES_FILE" ]; then
    echo "Error: Rules file not found at $RULES_FILE"
    exit 1
fi

echo "This script will:"
echo "1. Copy udev rules to $DEST_FILE"
echo "2. Add your user to the 'input' group"
echo "3. Reload udev rules"
echo ""
echo "Note: You will need to log out and log back in for group changes to take effect."
echo ""

# Check if running with sudo
if [ "$EUID" -ne 0 ]; then
    echo "This script requires sudo privileges. Please run with sudo:"
    echo "  sudo bash install_udev_rules.sh"
    exit 1
fi

# Get the actual user (not root when using sudo)
ACTUAL_USER="${SUDO_USER:-$USER}"

echo "Installing udev rules..."
cp "$RULES_FILE" "$DEST_FILE"
chmod 644 "$DEST_FILE"
echo "✓ Rules installed to $DEST_FILE"

echo ""
echo "Adding user '$ACTUAL_USER' to 'input' group..."
usermod -a -G input "$ACTUAL_USER"
echo "✓ User added to input group"

echo ""
echo "Reloading udev rules..."
udevadm control --reload-rules
udevadm trigger
echo "✓ udev rules reloaded"

echo ""
echo "Installation complete!"
echo ""
echo "IMPORTANT: You must log out and log back in for the group changes to take effect."
echo ""
echo "After logging back in, you can run JoyShockMapper from:"
echo "  ./build/JoyShockMapper/JoyShockMapper"

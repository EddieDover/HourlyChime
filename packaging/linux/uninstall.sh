#!/bin/bash
set -e

# Define paths
INSTALL_BIN="$HOME/.local/bin"
INSTALL_DESKTOP="$HOME/.local/share/applications"
INSTALL_ICON="$HOME/.local/share/icons/hicolor/128x128/apps"

echo "Uninstalling Hourly Chime..."

# Remove binary
if [ -f "$INSTALL_BIN/hourly_chime" ]; then
    echo "Removing binary..."
    rm "$INSTALL_BIN/hourly_chime"
else
    echo "Binary not found in $INSTALL_BIN"
fi

# Remove desktop file
if [ -f "$INSTALL_DESKTOP/hourly-chime.desktop" ]; then
    echo "Removing desktop entry..."
    rm "$INSTALL_DESKTOP/hourly-chime.desktop"
else
    echo "Desktop entry not found in $INSTALL_DESKTOP"
fi

# Remove icon
if [ -f "$INSTALL_ICON/hourly-chime.png" ]; then
    echo "Removing icon..."
    rm "$INSTALL_ICON/hourly-chime.png"
else
    echo "Icon not found in $INSTALL_ICON"
fi

# Update desktop database
if command -v update-desktop-database &> /dev/null; then
    update-desktop-database "$INSTALL_DESKTOP" || true
fi

echo "Uninstallation complete."

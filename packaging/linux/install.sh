#!/bin/bash
set -e

# Define paths
INSTALL_BIN="$HOME/.local/bin"
INSTALL_DESKTOP="$HOME/.local/share/applications"
INSTALL_ICON="$HOME/.local/share/icons/hicolor/128x128/apps"

# Ensure we are in the directory with the files
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Create directories
mkdir -p "$INSTALL_BIN"
mkdir -p "$INSTALL_DESKTOP"
mkdir -p "$INSTALL_ICON"

# Install binary
if [ -f "$DIR/hourly_chime" ]; then
    echo "Installing binary to $INSTALL_BIN..."
    cp "$DIR/hourly_chime" "$INSTALL_BIN/"
    chmod +x "$INSTALL_BIN/hourly_chime"
else
    echo "Error: hourly_chime binary not found in current directory."
    exit 1
fi

# Install icon
if [ -f "$DIR/icon.png" ]; then
    echo "Installing icon to $INSTALL_ICON..."
    cp "$DIR/icon.png" "$INSTALL_ICON/hourly-chime.png"
else
    echo "Warning: icon.png not found."
fi

# Install desktop file
if [ -f "$DIR/hourly-chime.desktop" ]; then
    echo "Installing desktop entry to $INSTALL_DESKTOP..."
    cp "$DIR/hourly-chime.desktop" "$INSTALL_DESKTOP/"
else
    echo "Error: hourly-chime.desktop not found."
    exit 1
fi

# Update desktop database
if command -v update-desktop-database &> /dev/null; then
    update-desktop-database "$INSTALL_DESKTOP" || true
fi

echo "Installation complete!"
echo "Ensure $INSTALL_BIN is in your PATH."

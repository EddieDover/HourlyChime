# Hourly Chime

A cross-platform system tray application that plays a sound on the hour.

## Features

- Runs in the system tray.
- Three chime modes:
  - **Notes**: Play a synthesized melody using simple note notation (e.g., "C E G").
  - **Audio File**: Play a specific audio file (MP3, WAV, OGG).
  - **Grandfather Clock**: Play a prelude (optional) followed by a strike file repeated for the current hour number.

## Development Prerequisites

### Linux (Fedora)
You will need the following development packages:
```bash
sudo dnf install libX11-devel libxdo-devel cairo-devel atk-devel cairo-gobject-devel gdk-pixbuf2-devel pango-devel gtk3-devel alsa-lib-devel
```

## Building and Running

To run the application in development mode:
```bash
cargo run
```

To build a release binary:
```bash
cargo build --release
```

The binary will be located in `target/release/hourly_chime`.

## Usage

1. Start the application. A tray icon will appear in your system tray.
2. Right-click the tray icon to access the menu.
3. Select "Settings" to configure the chime mode and sounds.
4. Select "Quit" to exit the application.

## Configuration

Configuration is stored in your system's standard configuration directory (e.g., `~/.config/HourlyChime/config.json` on Linux).

### Modes

- **Notes**: Enter a sequence of notes separated by spaces.
  - Supported notes: A-G, sharps (#), flats (b).
  - Octaves: Append a number (e.g., C4, A#5). Default is octave 4.
  - Rests: Use `-` to hold the previous note longer, or `X` for silence.
- **Audio File**: Select a single audio file to play on the hour.
- **Grandfather Clock**:
  - **Prelude**: An optional file played once before the strikes.
  - **Strike File**: The sound of a single clock strike.
  - **Strike Interval**: The time in milliseconds between the start of each strike. This allows for overlapping sounds (e.g., the previous strike decaying while the next one begins).


# Attributions

## Images

- Grandfather Clock Icon - <a href="https://www.flaticon.com/free-icons/grandfather-clock" title="grandfather clock icons">Grandfather clock icons created by Iconic Panda - Flaticon</a>

## Sounds

- Default Prelude and Chime - <a href="https://pixabay.com/sound-effects/grandfather-clock-strikes-ten-30067/">Grandfather clock strikes ten - Pixabay</a>
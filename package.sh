#!/bin/bash
set -e

echo "Building release binary..."
cargo build --release

echo "Creating distribution package..."
rm -rf dist
mkdir -p dist

cp target/release/hourly_chime dist/
cp packaging/linux/install.sh dist/
cp packaging/linux/uninstall.sh dist/
cp packaging/linux/hourly-chime.desktop dist/
cp assets/images/icon.png dist/

echo "Creating archive..."
tar -czf hourly_chime_linux.tar.gz -C dist .

echo "Done! Package created at hourly_chime_linux.tar.gz"

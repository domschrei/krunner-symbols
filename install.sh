#!/bin/bash

set -e

# Get correct distro-dependent installation directories 
loc_plugin=$(kf5-config --qt-plugins)
loc_desktop=$(kf5-config --path services|awk -F ':' '{print $NF}')
loc_config=$(kf5-config --prefix)/share/config

# Fetch and unpack packaged files
if [ ! -f krunner-symbols-*-Linux.tar.gz ]; then
    wget https://github.com/domschrei/krunner-symbols/releases/download/1.1.0/krunner-symbols-1.1.0-Linux.tar.gz
fi
pkg=$(echo krunner-symbols-*-Linux.tar.gz|awk '{print $NF}')
mkdir -p build
tar xzvf "$pkg" -C build --strip-components=1
cd build

# Fetch current version
version=$(grep X-KDE-PluginInfo-Version plasma-runner-symbols.desktop|grep -oE '[0-9]+\..*\..*')

# Install files
sudo cp krunner_symbols.so "$loc_plugin"/
sudo cp plasma-runner-symbols.desktop "$loc_desktop"/
sudo mkdir -p "$loc_config"
sudo cp krunner-symbolsrc krunner-symbols-full-unicode-index "$loc_config/"

echo "Installation successful. Restarting krunner for the changes to take effect."
if pgrep -x krunner > /dev/null; then
    killall krunner
fi

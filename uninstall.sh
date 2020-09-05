#!/bin/bash

set -e

# Get correct distro-dependent installation directories 
loc_plugin=$(kf5-config --qt-plugins)
loc_desktop=$(kf5-config --path services|awk -F ':' '{print $NF}')
loc_config=$(kf5-config --prefix)/share/config

# Remove installed files
sudo rm "$loc_plugin"/krunner_symbols.so
sudo rm "$loc_desktop"/plasma-runner-symbols.desktop
sudo rm "$loc_config"/krunner-symbolsrc 
sudo rm "$loc_config"/krunner-symbols-full-unicode-index

echo "Uninstallation successful. Restarting krunner for the changes to take effect."
if pgrep -x krunner > /dev/null; then
    killall krunner
fi

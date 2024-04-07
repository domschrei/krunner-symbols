#!/bin/bash

set -e

# Get correct distro-dependent installation directories 
loc_plugin=$(qtpaths --plugin-dir)
loc_config=$(qtpaths --install-prefix)/share/config

# Remove installed files
sudo rm "$loc_plugin/krunner_symbols.so"
sudo rm "$loc_config/krunner-symbolsrc"
sudo rm -rf "$loc_config/krunner-symbols-unicode"

echo "Uninstallation successful. Restarting krunner for the changes to take effect."
if pgrep -x krunner > /dev/null; then
    killall krunner
fi

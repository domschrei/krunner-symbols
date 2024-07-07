#!/bin/bash

set -e

# Get correct distro-dependent installation directories 
loc_plugin=$(qtdiag6 | grep PluginsPath | tr -d ' ' | cut -d ':' -f 2 )
loc_config=$(qtdiag6 | grep PrefixPath | tr -d ' ' | cut -d ':' -f 2)/share/config

# Remove installed files
sudo rm "$loc_plugin/krunner_symbols.so"
sudo rm "$loc_config/krunner-symbolsrc"
sudo rm -rf "$loc_config/krunner-symbols-unicode"

echo "Uninstallation successful. Restarting krunner for the changes to take effect."
if pgrep -x krunner > /dev/null; then
    killall krunner
fi

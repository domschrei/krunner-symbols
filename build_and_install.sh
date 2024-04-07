#!/bin/bash

# Exit immediately if something fails
set -e

# Setup build directory
mkdir -p build
cd build

# Get correct installation directories
prefix=$(qtdiag6 | grep PrefixPath | tr -d ' ' | cut -d ':' -f 2)
loc_plugin=$(qtdiag6 | grep PluginsPath | tr -d ' ' | cut -d ':' -f 2 |sed 's.^'"$prefix"'/..')
loc_config=share/config

# Build the plugin
cmake .. -DCMAKE_INSTALL_PREFIX=$prefix \
 -DLOCATION_PLUGIN=$loc_plugin \
 -DLOCATION_CONFIG=$loc_config \
 -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Install the plugin (root access because it has to write into /usr)
sudo make install

# Restart krunner
bash ../restart-krunner.sh

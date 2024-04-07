#!/bin/bash

# Exit immediately if something fails
set -e

# Setup build directory
mkdir -p build
cd build

# Get correct installation directories
prefix=$(qtpaths --install-prefix)
loc_plugin=$(qtpaths --plugin-dir|sed 's.^'"$prefix"'/..')
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

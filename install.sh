#!/bin/bash

# Exit immediately if something fails
set -e

# Setup build directory
mkdir -p build
cd build

# Build the plugin
cmake .. -DCMAKE_INSTALL_PREFIX=`kf5-config --prefix` -DQT_PLUGIN_INSTALL_DIR=`kf5-config --qt-plugins`
make 
# Install the plugin (root access because it has to write into /usr)
sudo make install

# Restart krunner for the changes to take effect
kquitapp5 krunner 2> /dev/null || echo "No krunner instance has been running."
kstart5 --windowclass krunner krunner > /dev/null 2>&1 &

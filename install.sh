#!/bin/bash

# Exit immediately if something fails
set -e

# Setup build directory
mkdir -p build
cd build

# Build the plugin
cmake .. -DCMAKE_INSTALL_PREFIX=`kf5-config --prefix` -DKDE_INSTALL_QTPLUGINDIR=`kf5-config --qt-plugins` -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
# Install the plugin (root access because it has to write into /usr)
sudo make install

# Restart krunner
bash ../restart-krunner.sh

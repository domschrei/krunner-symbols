#!/bin/bash

# Caution - This script is not guaranteed to work properly, if any complications occur. 
# In case of doubt, execute these steps one after one and watch the output.

mkdir -p build
cd build

cmake .. -DCMAKE_INSTALL_PREFIX=`kf5-config --prefix` -DQT_PLUGIN_INSTALL_DIR=`kf5-config --qt-plugins`
make 
sudo make install

# Copy the config file
cp ../krunner-symbols ~/.config/krunner-symbols

# Restart krunner for the changes to take effect
kquitapp krunner
krunner > /dev/null 2>&1 &

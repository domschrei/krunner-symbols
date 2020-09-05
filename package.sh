#!/bin/bash

# Exit immediately if something fails
set -e

# Set up clean build directory
mkdir build 2> /dev/null || rm -rf build/*
cd build

# Fetch current version
version=$(grep X-KDE-PluginInfo-Version ../plasma-runner-symbols.desktop|grep -oE "[0-9]+\..*\..*"|tr '.' ' ')
vmajor=$(echo $version|awk '{print $1}')
vminor=$(echo $version|awk '{print $2}')
vpatch=$(echo $version|awk '{print $3}')

package_flat_files=true
if $package_flat_files; then

    # Build the plugin in the build/ directory without any folder hierarchy
    cmake .. -DLOCATION_PLUGIN=. -DLOCATION_DESKTOP=. -DLOCATION_CONFIG=. \
    -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    
    # Package the plugin locally from the build/ directory
    make package

    # Copy files from build directory
    outfile="krunner-symbols-$vmajor.$vminor.$vpatch-Linux.tar.gz"
    cp "$outfile" ..
    cd ..

    echo
    echo "======================================"
    echo "Package created successfully: $outfile"
    echo "Install package by executing <bash install.sh> in this directory."

    
else
    # Get correct installation directories: relative paths without leading prefix.
    prefix=$(kf5-config --prefix) 
    loc_plugin=$(kf5-config --qt-plugins|sed 's.^'"$prefix"'/..')
    loc_desktop=$(kf5-config --path services|awk -F ':' '{print $NF}'|sed 's.^'"$prefix"'/..')
    loc_config=share/config

    # Build the plugin
    cmake .. -DCPACK_DEBIAN=true -DCMAKE_INSTALL_PREFIX=$prefix \
    -DLOCATION_PLUGIN=$loc_plugin \
    -DLOCATION_DESKTOP=$loc_desktop \
    -DLOCATION_CONFIG=$loc_config \
    -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    
    make package
    cp krunner-symbols-*.deb ..
fi

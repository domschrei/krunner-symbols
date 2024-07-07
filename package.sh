#!/bin/bash

# Exit immediately if something fails
set -e

if [ -z "$1" ]; then
    echo "Usage: $0 [debian|generic]"
    exit 1
fi

# Set up clean build directory
mkdir build 2> /dev/null || rm -rf build/*
cd build

# Fetch current version
version=$(jq -r '.KPlugin.Version' ../symbols.json|tr '.' ' ')
vmajor=$(echo $version|awk '{print $1}')
vminor=$(echo $version|awk '{print $2}')
vpatch=$(echo $version|awk '{print $3}')

if [ "$1" == "generic" ]; then
    # Build generic "flat" package to be installed via install.sh

    # Build the plugin in the build/ directory without any folder hierarchy
    cmake .. -DLOCATION_PLUGIN=. -DLOCATION_CONFIG=. \
    -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    
    # Package the plugin
    make package

    # Copy files from build directory
    outfile="krunner-symbols-$vmajor.$vminor.$vpatch-Linux.tar.gz"
    cp "$outfile" ..
    cd ..

    echo
    echo "======================================"
    echo "Package created successfully: $outfile"
    echo "Install package by executing <bash install.sh> in this directory."
    echo

else
    # Build DEB package

    # Get correct installation directories: relative paths without leading prefix.
    prefix=$(qtdiag6 | grep PrefixPath | tr -d ' ' | cut -d ':' -f 2)
    loc_plugin=$(qtdiag6 | grep PluginsPath | tr -d ' ' | cut -d ':' -f 2 |sed 's.^'"$prefix"'/..')
    loc_config=share/config

    # Build the plugin
    cmake .. -DCPACK_DEBIAN=true -DCMAKE_INSTALL_PREFIX=$prefix \
    -DLOCATION_PLUGIN=$loc_plugin \
    -DLOCATION_CONFIG=$loc_config \
    -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    
    # Package the plugin
    make package
    
    # Copy file from build directory
    outfile="krunner-symbols-$vmajor.$vminor.$vpatch.deb"
    cp "$outfile" ..
    
    echo
    echo "======================================"
    echo "Package created successfully: $outfile"
    echo "Install package by executing <sudo dpkg -i $outfile>."
    echo
fi

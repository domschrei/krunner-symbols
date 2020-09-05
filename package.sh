#!/bin/bash

# Exit immediately if something fails
set -e

# Setup build directory
mkdir -p build
cd build

# Fetch default install prefix and remove it from the plugin directory
install_prefix=$(kf5-config --prefix)
so_location=$(kf5-config --qt-plugins|sed 's.^'$install_prefix'..')

# Fetch current version
version=$(grep X-KDE-PluginInfo-Version ../plasma-runner-symbols.desktop|grep -oE "[0-9]+\..*\..*"|tr '.' ' ')
vmajor=$(echo $version|awk '{print $1}')
vminor=$(echo $version|awk '{print $2}')
vpatch=$(echo $version|awk '{print $3}')

# Build the plugin in the build/ directory
cmake .. -DKDE_INSTALL_QTPLUGINDIR=.$so_location -DCMAKE_BUILD_TYPE=Release -DCPACK_PACKAGE_VERSION_MAJOR=$vmajor -DCPACK_PACKAGE_VERSION_MINOR=$vminor -DCPACK_PACKAGE_VERSION_PATCH=$vpatch
make -j$(nproc)

# Package the plugin, installing it LOCALLY in the build/ directory
# (root access because of file owner issues)
sudo make package

# Move self-extracting archive to base directory
outfile="krunner-symbols-$vmajor.$vminor.$vpatch-Linux.sh"
cp $outfile ..
cd ..

echo "You can now install the plugin with the following command:"
echo sudo bash $outfile --prefix=$install_prefix --exclude-subdir --skip-license && bash restart-krunner.sh

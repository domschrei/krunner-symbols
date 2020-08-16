#!/bin/bash

# Exit immediately if something fails
set -e

cd build
sudo make uninstall
kquitapp5 krunner 2> /dev/null; kstart5 --windowclass krunner krunner > /dev/null 2>&1 &


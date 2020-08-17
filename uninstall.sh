#!/bin/bash

# Exit immediately if something fails
set -e

cd build
sudo make uninstall
kquitapp5 krunner
kstart5 --windowclass krunner krunner


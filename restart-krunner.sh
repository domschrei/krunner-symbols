#!/bin/bash
# Small utility to restart krunner.

# Restart krunner for the changes to take effect
kquitapp5 krunner 2> /dev/null || echo "No krunner instance has been running."
kstart5 --windowclass krunner krunner > /dev/null 2>&1 &

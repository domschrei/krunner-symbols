#!/bin/bash

set -e

if [ ! -d unicode-table-data ]; then
    echo "Fetching unicode-table-data."
    git clone https://github.com/domschrei/unicode-table-data
    cd unicode-table-data
else
    echo "Updating unicode-table-data."
    cd unicode-table-data
    git pull
fi

cd loc

if [ -z $1 ]; then
    locales=$(echo *)
    echo "Extracting all available locales ($locales)."
else
    locales="$@"
    echo "Extracting user-provided locales ($locales)."
fi

cd ../..

for loc in $locales; do
    echo "Extracting table for locale \"$loc\" ..."
    
    dir=unicode-table-data/loc/$loc/symbols
    out=krunner-symbols-unicode/unicode_$loc
    if [ ! -d "$dir" ]; then
        echo "Directory $dir does not exist! Exiting."
        exit 1
    fi

    echo "[Unicode]" > $out
    echo "" >> $out

    for f in $dir/*.txt $dir/plane*/*.txt ; do
        
        if [ ! -f "$f" ]; then continue; fi
        
        sed 's/\r//g' $f | grep -vE '^[0-9A-F]+: ?$' \
        |sed -r 's/^([0-9A-F]+): (.*)$/\2=\1/g' \
        | sed 's/\\//g' | sed 's/\[.*\]/.../g' >> $out

    done
    echo "Done."
done

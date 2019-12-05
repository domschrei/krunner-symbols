#!/bin/bash

set -e

wget http://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt -O UnicodeData.txt
echo "[Unicode]" > krunner-symbols-full-unicode-index
echo >> krunner-symbols-full-unicode-index
cat UnicodeData.txt|awk -F ';' '{print $2"="$1;}'|grep -vE '<.*>' >> krunner-symbols-full-unicode-index
rm UnicodeData.txt
#!/bin/bash
GREEN='\033[0;32m'
RED='\033[0;31m'
NOCOLOR='\033[0m'

set -e
#set -x
#alias nanoutimesstat="stat --printf=\"atime: %x\nmtime: %y\n\""


npm install
npm rebuild

touch file.txt

npm run nanoutimes file.txt 1524222671 123456789 1524222671 123456789
echo "test 1 - set atime and mtime to 4/20" > actual.txt
stat --printf="atime: %x\nmtime: %y\n" file.txt >> actual.txt

echo -e "\ntest 2 - increment mtime by 1s and 1ns" >> actual.txt
npm run nanoutimes file.txt 0 0 1524222672 123456790
stat --printf="atime: %x\nmtime: %y\n" file.txt >> actual.txt

echo -e "\ntest 3 - increment atime by 2s and 2ns" >> actual.txt
npm run nanoutimes file.txt 1524222673 123456791 0 0
stat --printf="atime: %x\nmtime: %y\n" file.txt >> actual.txt

rm file.txt

if [ `uname -o` = 'Msys' ]; then
  DIFF_FILENAME=expected-windows.txt
elif [ `uname -o` = 'GNU/Linux' ]; then
  DIFF_FILENAME=expected-linux.txt
fi

echo "DIFF_FILENAME: ${DIFF_FILENAME}"

if diff actual.txt $DIFF_FILENAME; then
  echo -e "${GREEN}Test passed${NOCOLOR}"
  exit 0
else
  echo -e "${RED}Test failed${NOCOLOR}"
  exit 1
fi

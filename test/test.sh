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
stat --printf="atime: %x\nmtime: %y\n" file.txt > actual.txt
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

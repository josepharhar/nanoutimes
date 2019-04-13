#!/bin/bash
set -e
set -x

#alias nanoutimesstat="stat --printf=\"atime: %x\nmtime: %y\n\""

npm install
npm rebuild

touch file.txt
npm run nanoutimes file.txt 1524222671 123456789 1524222671 123456789
stat --printf="atime: %x\nmtime: %y\n" file.txt > actual.txt
rm file.txt

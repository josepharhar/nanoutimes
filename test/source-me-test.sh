alias nanoutimesstat="stat --printf=\"atime: %x\nmtime: %y\n\""
c && npm rebuild && c && touch file.txt && nanoutimesstat file.txt && npm start && nanoutimesstat file.txt 

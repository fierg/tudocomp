#/bin/bash

FILE=$1

[ -e $FILE.tdc ] && rm $FILE.tdc

./tdc -a "mrle(coder=huff)" $FILE

#!/bin/bash
cd prueba

ORIGIN=$PWD

mkdir -p dir1/dir2/dir3 2> /dev/null
cd dir1/dir2/dir3 2> /dev/null

touch text.txt 2> /dev/null
RESULTADO=$?

if [ -f "text.txt" ] && [ $RESULTADO -eq 0 ]; then
    echo -e "Mult Dir: SUCCESS!"
    cd $ORIGIN
    rm -rf dir1 > /dev/null 2>&1
    exit 0
else
    echo -e "Mult Dir: FAIL!"
    cd $ORIGIN
    rm -rf dir1 > /dev/null 2>&1
    exit 1
fi

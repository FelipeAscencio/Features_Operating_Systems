#!/bin/bash
cd prueba

rm txt1.txt 2> /dev/null
RESULTADO=$?
rm txt2.txt 2> /dev/null
RESULTADO=$(( $RESULTADO + $? ))

if [ ! -f "txt1.txt" ] && [ ! -f "txt2.txt" ] && [ $RESULTADO -eq 0 ]; then
    echo -e "Delete File: SUCCESS!"
    exit 0
else
    echo -e "Delete File: FAIL!"
    exit 1
fi

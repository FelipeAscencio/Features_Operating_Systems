#!/bin/bash
cd prueba

echo prueba > file.txt

STAT_1=$(stat file.txt)
RESULTADO=$?

echo cambios > file.txt
RESULTADO=$(( $RESULTADO + $? ))

STAT_2=$(stat file.txt)
RESULTADO=$(( $RESULTADO + $? ))

if [ "$STAT_1" != "$STAT_2" ] && [ $RESULTADO -eq 0 ]; then
    echo -e "Stats Change: SUCCESS!"
    rm file.txt > /dev/null 2>&1
    exit 0
else
    echo -e "Stats Change: FAIL!"
    rm file.txt > /dev/null 2>&1
    exit 1
fi

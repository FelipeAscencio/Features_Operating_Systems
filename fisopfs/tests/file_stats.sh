#!/bin/bash
cd prueba

touch file.txt

STAT_1=$(stat file.txt)
RESULTADO=$?
STAT_2=$(stat file.txt)
RESULTADO=$(( $RESULTADO + $? ))

if [ "$STAT_1" == "$STAT_2" ] && [ $RESULTADO -eq 0 ]; then
    echo -e "File Stats: SUCCESS!"
    rm file.txt > /dev/null 2>&1
    exit 0
else
    echo -e "File Stats: FAIL!"
    rm file.txt > /dev/null 2>&1
    exit 1
fi

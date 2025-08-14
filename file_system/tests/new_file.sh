#!/bin/bash
cd prueba

touch txt1.txt 2> /dev/null
RESULTADO=$?
echo "hola mundo" > txt2.txt 2> /dev/null
RESULTADO=$(( $RESULTADO + $? ))

if [ -f "txt1.txt" ] && [ -f "txt2.txt" ] && [ $RESULTADO -eq 0 ]; then
    echo -e "Create File: SUCCESS!"
    exit 0
else
    echo -e "Create File: FAIL!"
    exit 1
fi
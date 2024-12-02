#!/bin/bash
cd prueba

mkdir dir 2> /dev/null
RESULTADO=$?

if [ -d "dir" ] && [ $RESULTADO -eq 0 ]  ; then
    echo -e "Dir Creation: SUCCESS!"
    exit 0
else
    echo -e "Dir Creation: FAIL!"
    exit 1
fi

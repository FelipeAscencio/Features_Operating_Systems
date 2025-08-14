#!/bin/bash
cd prueba

rmdir dir  2> /dev/null
RESULTADO=$?

if [ ! -d "dir" ] && [ $RESULTADO -eq 0 ] ; then
    echo -e "Dir Remove: SUCCESS!"
    exit 0
else
    echo -e "Dir Remove: FAIL!"
    exit 1
fi

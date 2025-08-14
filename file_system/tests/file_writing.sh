#!/bin/bash
cd prueba

echo "Esto es el inicio de un test." > text.txt 2> /dev/null
echo "Este test pasa con contenido adicional" >> text.txt 2> /dev/null

if grep -q "Esto es el inicio de un test." text.txt && grep -q "Este test pasa con contenido adicional" text.txt; then
    echo -e "File Writing: SUCCESS!"
    rm text.txt > /dev/null 2>&1
    exit 0
else
    echo -e "File Writing: FAIL!"
    rm text.txt > /dev/null 2>&1
    exit 1
fi

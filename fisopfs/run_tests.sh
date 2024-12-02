#!/bin/bash
ok=0
fail=0

pruebas=(
    "tests/new_file.sh"
    "tests/delete_file.sh"
    "tests/mult_dir.sh"
    "tests/file_writing.sh"
    "tests/dir_creation.sh"
    "tests/dir_rm.sh"
    "tests/file_stats.sh"
    "tests/stats_change.sh"
    "tests/lorem.sh"
)


for test in "${pruebas[@]}"; do
    ./$test
    if [[ $? -eq 0 ]]; then
        ok=$((ok + 1))
    else
        fail=$((fail + 1))
    fi
done

echo -e "\nTests\n"
echo -e "Passed: $ok"
echo -e "Failed: $fail"

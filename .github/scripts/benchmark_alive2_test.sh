#!/bin/bash

LLVM_DIR=/opt/llvm/bin
ALIVE_TV_BINARY=$(realpath "$1"/alive-tv)

cd swpp202301-benchmarks/

# Read the CSV file (skipping the header)
while IFS=, read -r _ CLASS_NAME PASS_NAME; do
    CLASS_NAMES+=("-load-pass-plugin=../build/lib${CLASS_NAME}.so")
    PASS_NAMES+=("$PASS_NAME")

    # Print the variables to check the values
    echo "Pass Name: $PASS_NAME"
    echo "Class Name: $CLASS_NAME"
    echo "---"
done < <(tail -n +2 ../unit_tests/entries.csv)

PASS_NAMES_STR=$(IFS=,; echo "${PASS_NAMES[*]}")

for dir in *; do
    if [ -d "${dir}/src" ]; then
        # Find the .ll file
        ll_file=$(find "${dir}/src" -name "*.ll")

        echo "Checking ${ll_file}..."

        # Run LLVM opt command with the appropriate arguments
        ${LLVM_DIR}/opt "${ll_file}" "${CLASS_NAMES[@]}" -passes="${PASS_NAMES_STR}" -S -o "${ll_file}.out" || exit 1

        # Run alive-tv command and capture the output
        alive_output=$("${ALIVE_TV_BINARY}" "${ll_file}" "${ll_file}.out" --quiet)

        echo "$alive_output"

        # Check the result and notify if there are errors
        if [ $? -ne 0 ] || \
           [[ ! $alive_output =~ "0 incorrect transformations" ]] || \
           [[ ! $alive_output =~ "0 Alive2 errors" ]]; then
            echo "ERROR"
        fi
    fi
done
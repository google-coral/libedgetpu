#!/usr/bin/env bash

set -euo pipefail

output="$1"
shift

declare -a srcs
srcs=("$@")

echo "namespace {" > "$output"

for file in "${srcs[@]}"; do
  filename="${file##*/}"
  filebase="${filename%.*}"
  {
    echo "const unsigned char ""$filebase""[] = {"
    xxd -i < "$file"
    echo "};"
    echo "constexpr unsigned int ""$filebase""_len = sizeof(""$filebase"") / sizeof(unsigned char);"
    echo ""
  } >> "$output"
done

echo "} // namespace" >> "$output"

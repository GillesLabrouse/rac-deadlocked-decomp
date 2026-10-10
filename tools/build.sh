#!/usr/bin/env bash
# Compile game C with the matching compiler into build/obj/.
# Add --incremental to reuse objects whose complete provenance is current.
#   bash tools/docker/run.sh bash tools/build.sh      (Linux/macOS)
# Compiler: SN GCC 2.95.3 v1.36 from the ProDG 3.01 mirror, -O2 -G8 -fopt-stack
# (SN-only flag: 8-byte register save slots, as retail; found by the UYA decomp).
set -euo pipefail
cd "$(dirname "$0")/.."
CC=toolchain/sn-prodg-3.01/usr/local/sce/ee/gcc/bin/ee-gcc2953.exe
export WINEDEBUG=-all
source tools/toolchain.sh
# An assignment propagates planner failure under set -e; a process
# substitution would hide it and could falsely report a successful build.
sources=$("$PYTHON" tools/plan_build.py "$@")
mkdir -p build/obj
n=0
while IFS= read -r f; do
    [ -n "$f" ] || continue
    o=build/obj/$(echo "${f#src/}" | tr '/' '_' | sed 's/\.c$/.o/')
    # a file may add flags with a first-lines comment:  /* cflags: -mno-split-addresses */
    extra=$(head -3 "$f" | tr -d '\r' | sed -n 's#^/\* cflags: \(.*\) \*/#\1#p')
    bash tools/cc.sh "$f" "$o" $extra
    n=$((n+1))
done <<< "$sources"
echo "compiled $n files into build/obj/"

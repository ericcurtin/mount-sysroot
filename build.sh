#!/bin/bash

set -e

gcc -O3 -Wall -Wextra -pedantic -fanalyzer -o mount-sysroot.gcc mount-sysroot.c -lblkid &
clang -O3 -Wall -Wextra -pedantic -o mount-sysroot.clang mount-sysroot.c -lblkid &
wait

mv mount-sysroot.gcc mount-sysroot &
rm mount-sysroot.clang &


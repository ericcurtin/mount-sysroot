#!/bin/bash

set -e

gcc -O3 -Wall -Wextra -pedantic -fanalyzer -o mountgpt.gcc mountgpt.c -lblkid &
clang -O3 -Wall -Wextra -pedantic -o mountgpt.clang mountgpt.c -lblkid &
wait

mv mountgpt.gcc mountgpt &
rm mountgpt.clang &


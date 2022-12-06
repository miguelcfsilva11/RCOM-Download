#!/bin/sh

# Stops in at first error.
set -xe
COMMON_CFLAGS="-Wall -Wextra -pedantic"
gcc $COMMON_CFLAGS ./$1.c -o $1


#!/bin/sh

# Stops in at first error.
set -xe
COMMON_CFLAGS="-Wall -Wextra -pedantic"

gcc $COMMON_CFLAGS -D PRINT_COMMUNICATION ftpPath.c ftpClient.c download.c -o download


#!/bin/bash

if [ $# -ne 2 ]
then
    echo "Usage: $0 src target"
    echo "Example: $0 wm.jpg wm.pgm"
    echo
    exit 1
fi
# pnmscale
jpegtopnm $1 | ppmtopgm >$2

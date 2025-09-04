#!/bin/sh

CURFILE=$(realpath "$0")
CURPATH=$(dirname "$CURFILE")
DEFAULT_BINARY="$CURPATH/../build/src/application/camera-control-code.hex"
BINARY="${1:-$DEFAULT_BINARY}"

BINARY=$(realpath "$BINARY")
echo "Using binary: $BINARY"
echo
echo

openocd -f interface/jlink.cfg -c 'transport select jtag; adapter speed 100'  -f board.cfg -c "program $BINARY verify reset exit"

#!/bin/sh
BINARY="../build/src/application/camera-control-code.hex"

openocd -f interface/jlink.cfg -f board.cfg -c "program $BINARY verify reset exit"

#!/bin/sh
BINARY="../build/src/application/camera-control-code.hex"

openocd -f interface/jlink.cfg -c 'transport select jtag; adapter speed 400'  -f board.cfg -c "program $BINARY verify reset exit"

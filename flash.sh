#!/bin/sh
BINARY="build/src/application/camera-control-code.hex"

openocd -f interface/jlink.cfg -c "transport select swd" -f target/stm32f4x.cfg -c "program $BINARY verify reset exit"

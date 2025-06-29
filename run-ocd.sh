#!/bin/sh
openocd -f interface/jlink.cfg -c "transport select jtag" -f target/stm32f4x.cfg


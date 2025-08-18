#!/bin/sh
openocd -f interface/jlink.cfg -c 'transport select jtag; adapter speed 400' -f board.cfg


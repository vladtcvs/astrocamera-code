#!/bin/sh
openocd -f interface/jlink.cfg -c 'transport select jtag; adapter speed 100' -f board.cfg


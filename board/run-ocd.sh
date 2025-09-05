#!/bin/sh
openocd -f interface/jlink.cfg -c 'transport select jtag;' -f board.cfg


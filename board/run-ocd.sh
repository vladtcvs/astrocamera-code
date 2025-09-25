#!/bin/sh
openocd -f ftdi.cfg -c 'transport select jtag;' -f board.cfg


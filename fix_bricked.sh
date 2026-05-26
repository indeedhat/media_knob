#!/bin/bash

echo "this is just instructions not a runnable script"
exit 1

# Connect pico probe to the bricked controller
# gnd -> gnd
# 3.3v -> vdo
# pin 2 -> clk
# pin 3 -> dio

# connect via debug probe
openocd -f interface/cmsis-dap.cfg -f target/nrf52.cfg

# in a seperate pane
telnet localhost 4444
> reset halt
> nrf mass_erase
> flash write_image erase /home/indeedhat/Downloads/nice_nano_bootloader-0.11.0_s140_6.1.1.hex

# success

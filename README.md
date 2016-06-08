# RaspberryPi-FDS132
Code to connect a FDS132 to a RaspberryPi. As a base, the code from Jan Panteltje (http://panteltje.com/panteltje/raspberry_pi_FDS132_matrix_display_driver/index.html) was used. 

The original code worked only on a Raspberry Pi 1. This code uses the code
from http://abyz.co.uk/rpi/pigpio/examples.html#Misc_code in order to detect
the version of the Raspberry Pi.  

This has been tested on the Rapsberry Pi B, Raspberry Pi 2 model B and Raspberry Pi 3 model B. 

The following table makes connecting the pins a bit easier. Newer models have 40-pins, older have 26 pins. 
The numbering on both connections is the same. 

| Pin          | GPIO pin | pin connection |
|---|---|---|
| strobe       | GPIO8  | 24 |
| data         | GPIO9  | 21 |
| clock        | GPIO11 | 23 |
| row select a | GPIO22 | 15 |
| row select b | GPIO23 | 16 |
| row select c | GPIO24 | 18 |

#!/usr/bin/python

import sys
from gpiozero import LED
from time import sleep

led = LED(17)

#if str(sys.argv[1]) == '2':
led.on() 
sleep(0.5)
	
#if str(sys.argv[1]) == '3':
#	led.off()

#while True:
#	sleep(1)
#	led.on()
#	sleep(1)
#	led.off()
#	sleep(1)

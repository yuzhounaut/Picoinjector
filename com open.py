# -*- coding: utf-8 -*-
"""
Created on Tue Jan 11 22:06:30 2022

@author: Feng
"""
import serial.tools.list_ports
import serial

ser = serial.Serial("COM1", 9600)
if not ser.isOpen():
    ser.open()
print('com1 is open', ser.isOpen())
print(ser.name)
ser.close()
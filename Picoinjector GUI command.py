# -*- coding: utf-8 -*-

import serial . tools . list_ports
import serial
from tkinter import *
from tkinter import ttk
import tkinter
import threading
from time import sleep
from GUI_Classes import MainWindow
master = Tk()
MainWindow(master)
arduinoData = MainWindow.Arduinodata
Plist = [] #empty l i s t
TotPuffDuration = 0
TotPuffNum = 0
TotPuffVol = 0
def read_from_port (ser) :
    global TotPuffDuration
    global TotPuffNum
    global TotPuffVol
    while True :
        global Plist
        if MainWindow.reset == 1: #check if rest button was pressed
            TopPuffNum = 0
            TotPuffVol = 0
            TotPuffDuration = 0
            print ("reset")
            MainWindow.reset = 0
        if len (Plist) >= 3: #once 3 values have been writen to list it will reset to empty
            Plist = []
        Arduinoread = arduinoData.readlines(2)#read incoming data
        Arduinodecode = Arduinoread[0].decode()
        Puff_length = Arduinodecode.rstrip()
        Plist.append(Puff_length) #create list
        
        if len(Plist) == 3: #only read list once it has been filled to 3
            if Plist [0] == '404': #dependent on 1st value different information is communicated
                MainWindow.v.set("Error: Too many clicks")
                sleep (5)
                MainWindow.v.set("Puff "+ Plist[1] + " ms")
        elif Plist [0] == '0': #updated puff duration
            MainWindow.v.set("Puff "+ Plist[1] + " ms")
                
        else:
            print (Plist) #labels change once puff is exicuted
            MainWindow.v.set("Puff "+ Plist[1] + " ms")
            TotPuffNum = TotPuffNum + 1
            TotPuffVol = TotPuffVol + int(Plist[2])
            TotPuffDuration = TotPuffDuration + int(Plist[1])
            MainWindow.PuffTim.set("Total Puff Length"+ str(TotPuffDuration)+ " ms")
            MainWindow.PuffNum.set("Total Puff Number"+ str(TotPuffNum))
            MainWindow.PuffVol.set("Total Puff Volume"+ str(TotPuffVol)+ " pL")
            
thread = threading.Thread(target=read_from_port, args=(arduinoData, ))
thread.start()
master.mainloop()

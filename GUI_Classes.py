# -*- coding: utf-8 -*-

from tkinter import *
from tkinter import ttk
import serial.tools.list_ports
import serial
from time import sleep

class MainWindow:
    def __init__(self, master):
        #Setup
        self.master = master
        master.title ("Pico-Injector GUI")
        
        #Get nessasary Port info
        ports = serial.tools.list_ports.comports() #make list of ports where arduino might be connected
        available_ports = []
        for p in ports :
            available_ports.append(p.device)
        print (available_ports) #just print to command window
        
        #Place port info into combo box
        self.cb = ttk.Combobox(master, values=available_ports)
        self.cb.pack( )
        self.cb.bind('<<ComboboxSelected>>')
        self.cb.current(0) #random initial port
        self.arduinoData = serial.Serial(self.cb.get(), 9600)
        # selects port where data will be sent
        MainWindow.Arduinodata = serial.Serial(self.cb.get(), 9600)
        
        #Switch Button
        self.switch_btn = Frame(master)
        self.switch_btn.pack()
        self.switch_var = IntVar()
        Single_button = Radiobutton(self.switch_btn, text="Single Puff", variable=self.switch_var,
                                    indicatoron=0, value=1, width=15)
        Sequance_button = Radiobutton(self.switch_btn,text="Sequance Mode", variable=self.switch_var,
                                      indicatoron=0, value=2, width=15)
        Single_button.pack(side="left")
        Sequance_button.pack(side="left")
        
        #Entry Box
        self.e = Entry(master)
        self.e.pack()
        entry = self.e.get()
        self.e.focus_set()
        
        #Buttons
        self.config_btn = Button(master, text="Config.", width=10, command=self.callback)
        self.config_btn.pack()
        self.reset_btn = Button(master, text="Reset", width=10, command=self.reset)
        self.reset_btn.pack()
        
        #Label
        MainWindow.v = StringVar()
        self.lab = Label(master, textvariable=MainWindow.v)
        self.lab.pack()
        MainWindow.PuffNum = StringVar()
        self.lab2 = Label(master, textvariable=MainWindow.PuffNum)
        self.lab2.pack()
        MainWindow.PuffTim = StringVar()
        self.lab3 = Label(master, textvariable=MainWindow.PuffTim)
        self.lab3.pack()
        MainWindow.PuffVol = StringVar()
        self.lab4 = Label(master, textvariable=MainWindow.PuffVol)
        self.lab4.pack()
    
    def callback(self): #function to send input data to arduino sleep (0.5)
        print("Port:", self.cb.get(), self.e.get())
        MainWindow.v.set("Puff" + self.e.get() + " ms")
        if self.switch_var.get() == 1: #dependent on button value. first value is changed
            datatowrite = "1," + self.e.get()
            self.arduinoData.write(datatowrite.encode())
        else:
            datatowrite = "2," + self.e.get()
            # 2 as first value will tell Arduino to write list to sequance list
            self.arduinoData.write(datatowrite.encode())
            print(len(datatowrite))
    
    def reset(self):
        MainWindow.reset = StringVar()
        MainWindow.reset = 1
    
    if __name__ == '__main__':
        obj = MainWindow()

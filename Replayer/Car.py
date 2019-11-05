from FrameHandler import ID02handler, ID03handler, ID0Chandler
import tkinter as tk

id02 = ID02handler()
id03 = ID03handler()
id0c = ID0Chandler()


class Car:
    __engineOn=False
    __EngineRpm=0
    __CoolantTemp=0
    __gear=0
    __brakeApplied=False
    __speed=0
    def __init__(self):
        pass


    def getData(self, id, bytes):
        if id == 2:
            id02.setData(id, bytes)
            self.__engineOn = id02.engineOn
            self.__EngineRpm = id02.rpm
            self.__CoolantTemp = id02.coolantTemp
        if id == 3:
            id03.setData(id, bytes)
            self.__gear = id03.gear
            self.__brakeApplied = id03.brake
        if id == 0x000c:
            id0c.setData(id, bytes)
            self.__speed = id0c.speedMph

    def printData(self):
        if (self.__engineOn == False):
            print("Ignition off!")
        elif (self.__EngineRpm == 0):
            print("Engine off!")
        else:
            print("Engine RPM: {0:4d}, Coolant: {1:3d}C. Gear: {2}. Brake?: {3}, Speed: {4} MPH".format(self.__EngineRpm, self.__CoolantTemp, self.__gear, self.__brakeApplied, self.__speed))
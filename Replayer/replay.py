import sys
import os
import datetime
import time
import threading
from Car import Car
import collections
from FrameHandler import idHandler

idh = idHandler()

car = Car()
lines = open("canbusC.dmp", "r").readlines()
ids=list()
for index, line in enumerate(lines):
    currtime=datetime.datetime.strptime(line.split(",")[0].split("=")[1], '%Y-%m-%d %H:%M:%S.%f')
    nextTime=0
    try:
        nextTime=datetime.datetime.strptime(lines[index+1].split(",")[0].split("=")[1], '%Y-%m-%d %H:%M:%S.%f')
    except Exception as e:
        break
    frameData=line.rstrip('\n').split(",")[1].split(":")
    frameIDStr = frameData[1].split("=")[1]
    frameID = int(frameIDStr)
    frameLen = int(frameData[2].split("=")[1])
    frameBytes = frameData[3:]
    frameBytes = [int(x, 16) for x in frameBytes]
    if frameID == 1048:
        idh.setData(frameID, frameBytes)
        print("{0} -> {1}".format(frameID,frameBytes))
    time.sleep((nextTime.timestamp() - currtime.timestamp()))
    
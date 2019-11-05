import sys
import os
import datetime
import time
import threading
from Car import Car


car = Car()
lines = open("canbus.dmp", "r").readlines()

lastTime = time
def updateThread():
    while True:
        car.printData()
        time.sleep(0.1)



x = threading.Thread(target=updateThread, args=())
x.start()
for index, line in enumerate(lines):
    currtime=datetime.datetime.strptime(line.split(",")[0].split("=")[1], '%Y-%m-%d %H:%M:%S.%f')
    nextTime=0
    try:
        nextTime=datetime.datetime.strptime(lines[index+1].split(",")[0].split("=")[1], '%Y-%m-%d %H:%M:%S.%f')
    except Exception as e:
        sys.exit(0)
    frameData=line.rstrip('\n').split(",")[1].split(":")
    frameID = int(frameData[1].split("=")[1])
    frameLen = frameData[2].split("=")[1]
    frameBytes = frameData[3:]
    frameBytes = [int(x, 16) for x in frameBytes]
    car.getData(frameID, frameBytes)
    time.sleep(nextTime.timestamp() - currtime.timestamp())
    
import serial
import sys
import datetime
port=sys.argv[1]
baud=sys.argv[2]

ser = serial.Serial(port, baud, timeout = 2)

f=open("canbus.dmp","w")
f.write("Time,ID(Hex),Data(Hex)\n")
while True:
    time=str(datetime.datetime.now())
    try:
        read=ser.readline().decode().strip('\r\n')
        print(read)
        print("TIME: {0}, DATA: {1}".format(time,read))
        f.write("TIME={0},{1}\n".format(time,read))
    except Exception as e:
        pass

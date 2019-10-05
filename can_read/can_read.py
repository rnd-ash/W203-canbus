import serial
import sys
import datetime
port=sys.argv[1]
baud=sys.argv[2]

ser = serial.Serial(port, baud, timeout = 2)

f=open("dump.csv","w")
f.write("Time,ID(Hex),Data(Hex)\n")
while True:
    time=str(datetime.datetime.now())
    try:
        read=ser.readline().decode().strip('\r\n')
        print(read)
        parts=str(read).split(",")
        id=parts[0]
        data=parts[1]
        print("TIME: {0}, ID: {1}, DATA: {2}".format(time,id,data))
        f.write("{0},{1},{2}\n".format(time,id,data))
    except Exception as e:
        pass

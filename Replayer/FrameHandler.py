class idHandler:
    __bytes=list()
    __binaryBits=list()
    def setData(self, id, bytes):
        self.__binaryBits.clear()
        self.__bytes = bytes
        for byte in bytes:
            bits = [int(n) for n in bin(byte)[2:].zfill(8)]
            self.__binaryBits += bits
    
    def getValueFromOffset(self, offset, numberOfBits) -> int:
        total = 0;
        for i in range (offset, offset + numberOfBits):
            total=(total << 1) | self.__binaryBits[i]
        return total


class ID02handler(idHandler):
    engineOn=False
    rpm=0
    coolantTemp=0
    def setData(self, id, bytes):
        super().setData(id, bytes)
        self.rpm = self.getValueFromOffset(16, 16)
        if self.rpm == 65535:
            self.engineOn = False
            self.rpm = 0
        else:
            self.engineOn = True
        self.coolantTemp = self.getValueFromOffset(32, 8) - 40


class ID03handler(idHandler):
    gear=0
    brake=False
    def setData(self, id, bytes):
        super().setData(id, bytes)
        self.gear = self.getValueFromOffset(40, 3)
        # 4 -> Drive (100)
        # 1 -> Park  (001)
        # 2 -> Reverse (101)
        
        brake = self.getValueFromOffset(6, 2)
        if (brake > 0):
            self.brake = True
        else:
            self.brake = False

class ID0Chandler(idHandler):
    speedMph=0
    def setData(self, id, bytes):
        super().setData(id, bytes)
        self.speedMph = self.getValueFromOffset(8, 8) * 5 / 8
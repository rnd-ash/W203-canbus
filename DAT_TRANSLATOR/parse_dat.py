import sys
import os
import struct
import codecs
from googletrans import Translator

#
# CANBUS .DAT Decoder file
#
# Generates descriptions of each CAN Message from XS Monitor data files.
#
# Usage: parse_dat.py <INPUT FILE> <OUTPUT FILE>
#
#
# TODO: Autogenerate target language descriptions via google translate from German text,
#       target language should be in arguments to program using ISO 2 letter codes

# Open the data and output file
file=open(sys.argv[1], "rb")
write = codecs.open(sys.argv[2], 'w', encoding='utf8')

translator = Translator(service_urls=["translate.google.com", "translate.google.co.kr",
                      "translate.google.at", "translate.google.de",
                      "translate.google.ru", "translate.google.ch",
                      "translate.google.fr", "translate.google.es"])

class can_frame_msg:
    __bytes__ = []
    __unit__ = None

    def __init__(self, bytes: bytearray):
        self.__bytes__ = bytes

    def read_range(self, len: int) -> bytearray:
        ret = self.__bytes__[0:len]
        self.__bytes__ = self.__bytes__[len:]
        return ret

    def read_until(self, targ: bytearray):
        curr = bytearray(len(targ))
        while curr != targ:
            curr = bytearray(curr[1:len(targ)])
            curr.append(self.read_range(1)[0])

    def sanitize_string(self, chars: bytearray) -> str:
        """
        Sanitizes byte array of any char's that are not ascii, then
        returns a string from the byte array
        """
        return chars.decode('iso-8859-1')

    def process(self):
        self.__msg_name__ = self.read_range(self.read_range(1)[0]).decode('iso-8859-1')
        self.__offset__ = int(self.read_range(1)[0])
        self.__length__ = int(self.read_range(1)[0])
        self.read_range(0xE)
        x = self.read_range(1)[0]
        try:
            if x == 0x00:
                try:
                    self.__msg_desc__ = self.sanitize_string(self.read_range(self.read_range(1)[0]))
                except UnicodeDecodeError:
                    self.__msg_desc__ = "ERROR"
            else:
                self.__unit__ = self.sanitize_string(self.read_range(x))
                self.__msg_desc__ = self.sanitize_string(self.read_range(self.read_range(1)[0]))
        except IndexError:
            self.__msg_desc__ = "ERROR"

    def get_msg_name(self):
        return self.__msg_name__
    
    def get_offset(self) -> int:
        return self.__offset__

    def get_len(self) -> int:
        return self.__length__
    
    def get_desc(self) -> str:
        if self.__unit__: # format differently if the description has a corresponding unit
            return "({0}) {1}".format(self.__unit__, self.__msg_desc__)
        else:
            return self.__msg_desc__

    def set_translated_text(self, trans: str):
        self.__msg_desc__ = trans





class can_frame_data:
    __bytes__ = []
    __msgs__ = []
    __byte_buffer__ = bytearray()
    def __init__(self, bytes: bytearray):
        self.__bytes__ = bytes
        self.__msgs__ = []
        self.__byte_buffer__ = bytearray()
        

    def read_range(self, len: int) -> bytearray:
        ret = self.__bytes__[0:len]
        self.__bytes__ = self.__bytes__[len:]
        return ret

    def read_until(self, targ: bytearray):
        curr = bytearray(len(targ))
        while curr != targ:
            curr = bytearray(curr[1:len(targ)])
            x = self.read_range(1)[0]
            self.__byte_buffer__.append(x)
            curr.append(x)

    def parse_frame(self):
        self.read_range(2)
        self.__ecu_name__ = self.read_range(self.read_range(1)[0]).decode('iso-8859-1')
        if len(self.__ecu_name__) < 2:
            self.__ecu_name__ = "UNKNOWN"
        self.read_range(1)
        self.__ecu_id__ = int(struct.unpack("<H", self.read_range(2))[0])
        self.read_range(4)
        while True:
            self.__byte_buffer__ = bytearray()
            try:
                self.read_until(bytearray([0x07, 0x80]))
                self.read_range(3)
                self.__msgs__.append(can_frame_msg(self.__byte_buffer__))
            except IndexError:
                self.__msgs__.append(can_frame_msg(self.__byte_buffer__))
                break
        self.__msgs__ = self.__msgs__[1:]
        self.__num_msgs__ = len(self.__msgs__)
        for m in self.__msgs__:
            m.process()

    def get_msgs(self) -> list:
        self.__msgs__.sort(key=lambda l:  l.get_offset())
        return self.__msgs__

    def get_name(self) -> str:
        """
        Returns name of ECU assosiated with message
        """
        return self.__ecu_name__

    def get_id(self) -> int:
        """
        Returns integer ID of the ECU's CAN broadcast
        """
        return self.__ecu_id__

    def get_num_msgs(self) -> int:
        """
        Returns number of sub messages within CAN frame
        """
        return self.__num_msgs__

    def translate(self, lang: str):
        if len(self.__msgs__) == 1:
            return
        to_translate = ""
        try:
            for i in self.__msgs__:
                to_translate += i.get_desc().encode('utf8').decode('utf8') + "\n"
            translated_text = translator.translate(to_translate, src="DE", dest="EN").text
            for pos, line in enumerate(translated_text.split("\n")):
                try:
                    self.__msgs__[pos].set_translated_text(line)
                except IndexError:
                    continue
        except Exception:
            print("Error translating {0}".format(self.__ecu_name__))
            pass
        



can_frames = []
buffer=bytearray()
while True:
    x = file.read(1)
    if len(x) == 0: # Completed reading dat file. Add last frame to list and then process
        can_frames.append(buffer)
        break
    buffer += x
    if buffer.endswith(bytearray([0x05, 0x80])): # Identifier for next can frame!
        can_frames.append(buffer)
        buffer = bytearray()


print("Found {0} can frames. Decoding".format(len(can_frames)))
for f in can_frames:
    x = can_frame_data(f)
    x.parse_frame()
    #x.translate('en')
    print("ECU NAME: {0}, ID: {1}. MSG COUNT: {2}".format(x.get_name(), "0x%04X" % x.get_id(), x.get_num_msgs()))
    write.write("ECU NAME: {0}, ID: {1}. MSG COUNT: {2}\n".format(x.get_name(), "0x%04X" % x.get_id(), x.get_num_msgs()))
    for i in x.get_msgs():
        print("\tMSG NAME: {0} - {1}, OFFSET {2}, LENGTH {3}".format(i.get_msg_name(), i.get_desc(), i.get_offset(), i.get_len()))
        desc = i.get_desc()
        write.write("\tMSG NAME: {0} - {1}, OFFSET {2}, LENGTH {3}\n".format(i.get_msg_name(), desc, i.get_offset(), i.get_len()))

#!/usr/bin/env python3
import sys
import operator
from functools import reduce
filename = sys.argv[1]

hob = bin

def reverse_bit(num):
    return int('{:08b}'.format(num)[::-1], 2)

def parse_message(message):
    senderBit= message[0] >> 7
    isSender = senderBit == 1
    header_payload = message[0] & 0b01111111
    payload = message[1:-1]
    if(isSender):
        print(" New-Sender: " + hob(header_payload))
    elif(len(payload) >= 0):
        print("Data-Packet: " + hob(header_payload) + " \tdata: " + str([hob(p) for p in payload]))
        #zxor = header_payload ^ reduce(operator.xor, payload, 0)
        #print("Data-Packet: " + hob(header_payload) + " [ " + str([hob(p) for p in payload]) + " ] checksum " + hob(zxor) + " = " + hob(message[-1]))

with open(filename, "rb") as f:
    line = ""
    message = list()
    while True:
        ninebits = reverse_bit(int.from_bytes(f.read(2),'big'))
        if ninebits == b'':
            break
        parity = ninebits & 1
        byte = ninebits >> 1
        #print(bin(parity), bin(byte), bin(ninebits))
        if parity == 1:
            if len(message) > 0:
                parse_message(message)
            message = [byte]
        elif parity == 0:
            message.append(byte)
        else:
            print("Parity Error: 0x" + parity.hex() )


#!/usr/bin/env python3
import sys
import operator
import pprint
from functools import reduce
filename = sys.argv[1]

hob = bin

def reverse_bit(num):
    return int('{:08b}'.format(num)[::-1], 2)

def parse_message(message):
    senderBit= message[0] >> 7
    isSender = senderBit == 1
    header_payload = message[0] & 0b01111111
    payload = message[1:]
    if(isSender):
        print(" New-Sender: " + hob(header_payload))
    elif(len(payload) >= 0):
        print("Data-Packet: " + hob(header_payload) + " \tdata: " + str([hob(p) for p in payload]))
        #zxor = header_payload ^ reduce(operator.xor, payload, 0)
        #print("Data-Packet: " + hob(header_payload) + " [ " + str([hob(p) for p in payload]) + " ] checksum " + hob(zxor) + " = " + hob(message[-1]))


hobarray = lambda x: [hob(p) for p in x]
messagetypes = {}
messagelen = {}

def chosenMessageId(messageId):
    if (len(sys.argv)>2):
        return messageId == int(sys.argv[2])
    else:
        return True

def parse_data(message):
    messageid = message[0]
    if(messageid in messagetypes):
        messagetypes[messageid] += 1
        messagelen[messageid].append(len(message))
    else:
        messagetypes[messageid] = 1
        messagelen[messageid] = [len(message)]
    payload = message[1:-1]
    zxor = reduce(operator.xor, message[:-1], 0)
    if chosenMessageId(messageid):
        chksum = message[-1]==zxor
        info = "Unknown message [" + str(messageid) + "]: " + str(hobarray(payload))
        if(not chksum):
            info += "\tChksum failed: (orig!=calc) " + hob(message[-1]) + " = " + hob(zxor) + " " + str(chksum)
        print(info)    
        #print("Data-Packet: " + hob(header_payload) + " [ " + str([hob(p) for p in payload]) + " ] checksum " + hob(zxor) + " = " + hob(message[-1]))

with open(filename, "rb") as f:
    line = ""
    message = list()
    while True:
        twobytes = f.read(2)
        if twobytes == b'':
            break
        ninebits = int.from_bytes(twobytes,'big')
        parity = ninebits >> 8
        byte = reverse_bit(ninebits & 0xFF)
#        print(bin(parity), bin(byte), bin(ninebits))

        senderBit= byte >> 7
        isSender = senderBit == 1
        header_payload = byte & 0b01111111
        if (parity == 1):
            if (len(message)>0):
                parse_data(message)
            message = []
            if(isSender):
                print(" New-Sender: " + hob(header_payload))
            else:
                message.append(byte)
        else:
            message.append(byte)

    pp = pprint.PrettyPrinter(indent=4)
    pp.pprint(messagetypes)
    #pp.pprint(messagelen)
    print(str([hob(a) for a in messagetypes.keys()]))
            
        #if parity == 1:
        #    if len(message) > 0:
        #        parse_message(message)
        #    message = [byte]
        #elif parity == 0:
        #    message.append(byte)
        #else:
        #    print("Parity Error: 0x" + parity.hex() )


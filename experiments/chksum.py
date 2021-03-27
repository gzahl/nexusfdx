#!/usr/bin/env python3

message = ['0b00010101', '0b01100000', '0b01110000', '0b01111111', '0b001111111', '0b00011010'] 
message = [int.from_bytes(i, 'little') for i in message]

print("Chksum: " + bin(message[-1]))
print("Binary: " + bin(sum(message[:-1])))


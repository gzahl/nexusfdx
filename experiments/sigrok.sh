#!/usr/bin/env bash
sigrok-cli -i $1 -P uart:baudrate=9600:data_bits=9:format=bin -B uart=tx 

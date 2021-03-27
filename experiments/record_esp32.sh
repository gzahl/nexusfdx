#!/usr/bin/env bash
(stty raw 115200; cat > $2) < $1

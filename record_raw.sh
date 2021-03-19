#!/usr/bin/env bash
(stty raw parenb parmrk -cmspar cs8 cstopb 9600; cat > $2) < $1

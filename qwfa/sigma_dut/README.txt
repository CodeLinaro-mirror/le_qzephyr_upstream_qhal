#===============================================================================
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear
#===============================================================================

usage: python sigma_dut.py [-h] -ip IP -p PORT -S INTERFACE -s SERIAL_PORT [-t TEST] [-v]

options:
    -h, --help      show this help message and exit
    -ip IP          sigma control agent IP address, local IP address of PC running this tool.
    -p PORT         sigma control agent port, the port number running this tool.
    -S INTERFACE    DUT interface: wlan1
    -s SERIAL_PORT  DUT serial port like: COM6
    -t TEST         WFA cert test item: 11n
    -v, --version   show program's version number and exit
How to run 11n certification:
    python sigma_dut.py -ip 192.168.250.41 -p 9000 -S wlan1 -s com17 -t 11n


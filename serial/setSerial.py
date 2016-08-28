#!/usr/bin/python
# vi:si:et:sw=4:sts=4:ts=4
# -*- coding: UTF-8 -*-
# -*- Mode: Python -*-
#
# Copyright (C) 2006 Bertera Pietro <pietro@bertera.it>

# This file may be distributed and/or modified under the terms of
# the GNU General Public License version 2 as published by
# the Free Software Foundation.
# This file is distributed without any warranty; without even the implied
# warranty of merchantability or fitness for a particular purpose

import serial, getopt, sys, time

def usage():
    print sys.argv[0]+" -d 0|1 setta DTR al valore passato -r 0|1 setta RTS al valore passato -p porta"
    sys.exit(1)

def main():
	try:
        opts, argv = getopt.getopt(sys.argv[1:],"d:r:p:")
    except getopt.GetoptError:
        usage()
        sys.exit(1)
        
    rts = None
    dtr = None
    port = None
    
    for o, a in opts:
        if o == "-d":
            dtr = bool(int(a))
            if (dtr != 0) and (dtr != 1):
                usage()
        elif o == "-r":
            rts = bool(int(a))
            if (rts != 0) and (rts != 1):
                usage()
        elif o == "-p":
            port = a

    if rts == None or dtr == None or port == None:
        usage()
    
    try:
        s = serial.Serial(port)
        s.setDTR(dtr)
        s.setRTS(rts)
        time.sleep(1)
    except Exception, e:
        print e
        sys.exit(1)
    
if __name__ == "__main__":
	main()

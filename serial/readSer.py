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
# warranty of merchantability or fitness for a particular purpose.

import serial, time, sys, getopt

DELAY=0.001

def usage():
    print sys.argv[0]+" -p /dev/ttySx [-d sec]"

def main():
    try:
        opts, argv = getopt.getopt(sys.argv[1:],"p:d:")
    except getopt.GetoptError:
        usage()
        sys.exit(1)
    
    port = None
    delay = DELAY
    
    for o,a in opts:
        if o == '-p':
            port = a
        elif o == '-d':
            delay = float(a)
    
    if port == None:
        usage()
    
    start = time.time()
    print "Opening "+port

    try:
        ser = serial.Serial(port)
    except Exception, e:
        print e
        sys.exit(1)
    
    cts = ser.getCTS()
    dsr = ser.getDSR()

    while 1:
        try:
        	c = ser.getCTS()
        	if cts != c:
        		cts = c 
        		print "%f - cts: %d" % (time.time()-start , int(cts))
        	d = ser.getDSR()
        	if dsr != d:
        		dsr = d
        		print "%f - dsr: %d" % (time.time()-start, int(dsr))
            time.sleep(delay)
            ser.flushInput()
        except Exception, e:
            sys.exit(0)

if __name__ == "__main__":
    main()

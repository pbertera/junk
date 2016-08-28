# -*- Mode: Python -*-
# vi:si:et:sw=4:sts=4:ts=4
#
# Copyright (C) 2008 <pietro@bertera.it>

# This file may be distributed and/or modified under the terms of
# the GNU General Public License version 2 as published by
# the Free Software Foundation.
# This file is distributed without any warranty; without even the implied
# warranty of merchantability or fitness for a particular purpose.

import _winreg, getopt, sys
import PyRegedit as pyr

def usage():
    print "-h       :           help"
    print "-k <key> :           key to read"
    print "-r <host>:           remote host"
    sys.exit(1)

def read_remote_key(host, key):

    try:
        registry = pyr.PyRegedit(key, host=host)
    except Exception, e:
        print "Error opening remote registry"
        print e
        sys.exit(1)
    
    for v in registry.listValues():
        if v != "":
            print "%s: %s" % (v, registry.getValue(v)['value'])
        

if __name__ == "__main__":
    try:
        opts, args = getopt.getopt(sys.argv[1:], "k:r:h")
    except getopt.GetoptError:
        usage()

    key = None
    host = None
    
    for o,a in opts:
        if o == "-k":
            key = a
        elif o == "-r":
            host = a
        else:
            usage()
            sys.exit(1)
    
    if key == None:
        print "Error: key is required"
        sys.exit(1)
    
    if host == None:
        print "Error: remote host is required"

    read_remote_key(host, key)

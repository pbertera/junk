# -*- Mode: Python -*-
# vi:si:et:sw=4:sts=4:ts=4
#
# Copyright (C) 2007 Xsec S.r.l. <pietro.bertera@xsec.it>

# This file may be distributed and/or modified under the terms of
# the GNU General Public License version 2 as published by
# the Free Software Foundation.
# This file is distributed without any warranty; without even the implied
# warranty of merchantability or fitness for a particular purpose.

import _winreg as wreg
import sys, getopt, struct
import PyRegedit as pyr

def decode_prod_key(digital_product_id):
    key_start_index = 52
    key_end_index = key_start_index + 15
    digits = ['B', 'C', 'D', 'F', 'G', 'H', 'J', 'K', 'M', 'P', 'Q', 'R', 'T', 'V', 'W', 'X', 'Y', '2', '3', '4', '6', '7', '8', '9']
    decode_length = 29
    decode_string_length = 15
    decoded_chars = [x for x in range(1,decode_length + 1)]
    hex_product_id = []

    i = key_start_index

    while i <= key_end_index:
        hex_product_id.append(int(digital_product_id[i]))
        i = i +1
    
    i = decode_length -1
    while i >= 0:
        if ((i + 1) % 6) == 0:
            decoded_chars[i] = '-'
        else:
            digit_map_index = 0

            j = decode_string_length - 1
            while j >= 0:
                byte_value = (digit_map_index << 8 | hex_product_id[j])
                hex_product_id[j] = byte_value / 24
                digit_map_index = byte_value % 24
                decoded_chars[i] = digits[digit_map_index]
                j  = j - 1

        i = i - 1
    
    return "".join("%s" % i for i in decoded_chars)

def usage():
    
    print "-h        :      help"
    print "-c   <key>:      copy in cleartext to an other registry key"
    print "-r  <host>:      remote host to query registry"
    sys.exit(1)

def open_registry(data_path, host):

    try:
        registry = pyr.PyRegedit(data_path, host=host)
    except Exception, e:
        print "Error opening registry"
        print e
        sys.exit(1)

    return registry

def get_prod_id_from_key(key, host):
    
    registry = open_registry(key, host=host)

    try:
        digital_product_id = registry.getValue("digitalproductid")['value']
    except Exception, e:
        print "Error reading DigitalProductId"
        print e
        sys.exit(1)

    try:
        product_id = decode_prod_key(struct.unpack('164B', digital_product_id))
    except Exception, e:
        print "Error decoding key"
        print e
        sys.exit(1)

    return product_id

def get_office_keys(base, host):
    
    registry = open_registry(base, host=host)
    sub_keys = registry.listKeys()
    path = registry.DATA_PATH
    ret = []
    for v in ["9.0", "10.0", "11.0", "12.0"]:
        if v in sub_keys:
            b = path + "\\" + v
            registry = open_registry(b, host=host)
            if "registration" in registry.listKeys():
                registry = open_registry(b + "\\Registration", host=host)
                guid = registry.listKeys()[0]
                registry = open_registry(b + "\\Registration\\" + guid, host=host)
                
                if "digitalproductid" in registry.listValues():
                    digital_product_id = registry.getValue("digitalproductid")['value']
                    office_prod_id = decode_prod_key(struct.unpack('164B',digital_product_id))
                    if v == "9.0":
                        # Office 2000
                        office_prod_id = decode_prod_key(struct.unpack('164B',digital_product_id))
                        ret.append(("2000", office_prod_id))
                    elif v == "10.0":
                        # Office XP
                        office_prod_id = decode_prod_key(struct.unpack('164B',digital_product_id))
                        ret.append(("XP", office_prod_id))
                    elif v == "11.0":
                        # Office 2003
                        office_prod_id = decode_prod_key(struct.unpack('164B',digital_product_id))
                        ret.append(("2003", office_prod_id))
                    elif v == "12.0":
                        # Office 2007
                        office_prod_id = decode_prod_key(struct.unpack('164B',digital_product_id))
                        ret.append(("2007", office_prod_id))
    return ret

if __name__ == '__main__':

    try:
        opts, args = getopt.getopt(sys.argv[1:], "c:r:h")
    except getopt.GetoptError:
        usage()

    src_key = ""
    dst_val = None 
    copy = False
    host = None
    
    for o,a in opts:
        if o == '-c':
            copy = True
            dst_key = a
        elif o == '-r':
            host = a
        else:
            usage()
            sys.exit(1)

    windows_digital_product_id_key = r"SOFTWARE\Microsoft\Windows NT\CurrentVersion"
    win_prod_id = get_prod_id_from_key(windows_digital_product_id_key, host)
    office_base_key = r"SOFTWARE\Microsoft\Office" 
    office_keys = get_office_keys(office_base_key, host)
    
    if copy == True:
        
        registry = open_registry(dst_key, host=host)
        
        try:
            registry.setValueTxt("WINDOWS-KEY", win_prod_id)
            if len(office_keys) > 0:
                for k in office_keys:
                    registry.setValueTxt("OFFICE-%s-KEY" % k[0], k[1])
        except Exception, e:
            print "Error setting key"
            print e
            sys.exit(1)
    else:
        print  "WINDOWS KEY: " + win_prod_id
        if len(office_keys) > 0:
            for k in office_keys:
                print  "OFFICE %s KEY: %s" % (k)
        sys.exit()

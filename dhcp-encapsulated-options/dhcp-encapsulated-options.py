#!/usr/bin/python
# vi:si:et:sw=4:sts=4:ts=4
# -*- coding: UTF-8 -*-
# -*- Mode: Python -*-
#
# Pietro Bertera <pietro@bertera.it>

import sys

part_number = 0
full_encoded_options = ""

def error(message=None):
	if message: print "ERROR: %s" % message
	print "RFC2132 Encapsulated DHCP options encoding / decoding script"
	print
	print "Usage:"
	print "\t%s [encode | decode] args" % sys.argv[0]
	print "\t\t encode command args must be the encoded Vendor encapsulated Option"
	print "\t\t decode command args must be a list of couple OptionID Option value"
	print
	print "Example:"
	print "\t Decoding the vendor encapsulate option 42:17:68:74:74:70:3A:2F:2F:70:72:6F:76:2E:65:78:61:6D:70:6C:65:2E:63:6F:6D"
	print "\t Option number: 66"
	print "\t Option value: http://prov.example.com"
	print
	print "\t%s decode 42:17:68:74:74:70:3A:2F:2F:70:72:6F:76:2E:65:78:61:6D:70:6C:65:2E:63:6F:6D" % sys.argv[0]
	print
	print "\t Encoding option 66 with value http://prov.example.com and option 67 with value phonesettings.xml"
	print
	print "\t%s encode 66 http://prov.example.com 67 phonesettings.xml" % sys.argv[0]
	print
	print "\t Encapsulated DHCP options are defined by RFC2132"
	sys.exit(-1)

def dec2hex(n):
	try:
		return "%02X" % n
	except Exception:
		error("Error converting decimal to hex " + n)

def hex2dec(s):
	try:
		return int(s, 16)
	except Exception:
		error("Error converting hex to decimal " + s)

def hex2char(s):
	try:
		char = hex2dec(s)
		return chr(char)
	except Exception:
		error("Error converting hex to car " + s)

def decode(parts):
	global part_number
	part_number = part_number + 1
	opt_num = hex2dec(parts[0])
	opt_length = hex2dec(parts[1])
	opt_value_tuple=parts[2:opt_length+2]
	opt_value = "".join(map(hex2char, opt_value_tuple))
	
	print "Decoding part %d: %s:%s:%s" % (part_number, parts[0], parts[1], ":".join(opt_value_tuple))
	print " Option ID: %d" % opt_num
	print " Option lenght: %d" % opt_length
	print " Option value: \"%s\"" % opt_value
	print 
		
	if opt_length + 2 == len(parts):
		print "End of encoded options"
		return
		
	decode(parts[opt_length+2:])	

def encode(args):
	global part_number, full_encoded_options
	part_number = part_number + 1
	
	if len(args) < 2:
		error("encoding options mismatch")
	try:	
		opt_num = int(args[0])
	except ValueError:
		error("%s isn't an integer number" % args[0])
 
	opt_value = args[1]
	opt_length = len(opt_value)
	opt_value_dec = map(ord, opt_value)	
	opt_value_hex = map(dec2hex, opt_value_dec)	
	encoded_opt = "%s:%s:%s" % (dec2hex(opt_num), dec2hex(opt_length), ":".join(opt_value_hex))
	full_encoded_options = full_encoded_options + ":" + encoded_opt
		
	print "Encoding part %d: %s" % (part_number, opt_value)
	print " Option ID: %d" % opt_num
	print " Option length: %d" % opt_length
	print " Encoded Option part: %s" % encoded_opt
	print
	
	if len(args) - 2 == 0:
		print "Full encoded options: %s" % full_encoded_options[1:]
		return
	
	encode(args[2:])

def main():

	if len(sys.argv) < 2:
		error()
	if sys.argv[1] == "decode":
		try:
			decode(sys.argv[2].split(":"))		
		except IndexError:
			error("encoded option %s in wrong format" % sys.argv[2])

	elif sys.argv[1] == "encode":
		encode(sys.argv[2:])		

	else:
		error("Wrong command. Command must be 'encode' or 'decode'")
		sys.exit(-1)

if __name__ == "__main__":
	try:
		main()
	except Exception, e:
		error("Error occourred: %s" % e)

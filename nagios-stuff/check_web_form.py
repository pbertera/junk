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
# 
# Check web form
# Permette di effettuare il submit di un form HTML e di cercare dei contenuti nella
# response del server
# 
# Features:
#
# * Supporto per i Cookie
# * URL personabilizzabile per ricevere i cookie
# * Invio di Header HTTP arbitrari
# * Ricerca nel contenuto e nell'header della response
#
# Esempio:
#
# ./check_web_form -a http://www.spam.com/login.php -i username=CiccioUser -i password=CiccioUser -C "^Logged in as:  CiccioUser"
# Esegue il submit di un form con action="http://www.spam.com/login.php" e passa i campi username=CiccioUser e password=CiccioUser
# e controlla che nel risultato ci sia una riga che inizia con "Logged in as:  CiccioUser"
#
# ./check_web_form -a http://www.spam.com/login.php -i username=user -i password=Passw -H content-type=application/x-www-form-urlencoded -H "User-Agent=Mozilla" -m POST -c -u http://www.spam.com/smolla_il_cookie -R location=http://www.spam.com/*authenticated
#
# * Prende il cookie all' URL http://www.spam.com/smolla_il_cookie (-u)
# * Tramite metodo POST invia gli header content-type=application/x-www-form-urlencoded e User-Agent=Mozilla (-H)
# * Invia i valori username e password (-i)
# * L'invio viene effettuato all'URL http://www.spam.com/login.php (-a)
# * La ricerca del risultato avviene negl'header location della risposta (-R)

# Se l'esito ha buon fine ritorna 0 e stampa OK ... 
# Se fallisce ritorna 2 e stampa ERROR ...

import optparse, sys, re
from httplib2 import Http
from urllib import urlencode

class FormSubmitterOptionParser (optparse.OptionParser):

    def check_required (self, opt):
        option = self.get_option(opt)
        if getattr(self.values, option.dest) is None:
            self.error("%s option not supplied" % option)

    def check_method(self):
        method = self.get_option('--method')
        if getattr(self.values, method.dest).upper() not in ('GET', "POST"):
            self.error("HTTP method not implemented use GET or POST")
    
    def check_input(self):
        input = self.get_option('--input')
        for e in getattr(self.values, input.dest):
            if len(re.split(r'\s*=\s*', e, 1)) != 2:
                self.error("%s must be in format \"name=value\": %s" % (input, e))

if __name__ == '__main__':
    parser = FormSubmitterOptionParser("usage: %prog [options]")
    parser.add_option('-m', '--method', default='GET', type="string", help="HTTP method (GET or POST), default %default")
    parser.add_option('-i', '--input', action='append', help="input fileld name=value")
    parser.add_option('-a', '--action', action='store', help="Action of form")
    parser.add_option('-c', '--cookie', action='store_true', default=False, help="Support for cookie")
    parser.add_option('-u', '--cookie-url', action='store', help="URL used to retrieve cookie")
    parser.add_option('-H', '--header', action='append', help="Send header hdeadername=HeaderValue")
    parser.add_option('-C', '--content-match', action='store', help='Check for regular expression match in content')
    parser.add_option('-R', '--response-match', action='store', help='Check for regular expression match in response ex: Status=^4??', default=None)
    parser.add_option('-d', '--debug', action="store_true", default=False, help="Debug mode")
    
    (options,args)=parser.parse_args()

    try:
        # Check required options
        parser.check_required('-a')
        # parser.check_required('-i')
        # check http method
        parser.check_method()
        # check values
        if options.input:
           parser.check_input()
    
    except  AttributeError, e:
        print "Error: %s" % e
        sys.exit(255)
    
    # make header dict
    headers = {}
    if options.header != None:
        for val in options.header:
            name = re.split(r'\s*=\s*', val, 1)[0]
            value = re.split(r'\s*=\s*', val, 1)[1]
            headers.update({name: value})

    # make data dict
    fields = {}
    if options.input:
        for val in options.input:
            name = re.split(r'\s*=\s*', val, 1)[0]
            value = re.split(r'\s*=\s*', val, 1)[1]
            fields.update({name: value})

    http = Http()
    #headers = None
    
    # check in response Header
    if options.response_match != None:
        response_name = re.split(r'\s*=\s*', options.response_match, 1)[0] 
        response_value = re.split(r'\s*=\s*', options.response_match, 1)[1] 
    
    # get page to read cookie
    if options.cookie :
        if options.debug:
            print "Sending request to retrieve cookie:" 
            print "URL: %s" % options.action
            print "Method: %s" % options.method
            print "Headers: %s" % headers
            print ""

        if options.cookie_url:
            url = options.cookie_url
        else:
            url = options.action
        resp, content = http.request(url , options.method, headers=headers)
        
        if options.debug:
            print ""
            print "Response: %s" % resp
            print "Content: %s" % content
            print ""
    
        if resp.has_key('set-cookie'):
            headers.update({'Cookie': resp['set-cookie']})
        
        
    try:
        if options.method.upper() == 'GET':
            from urlparse import urlsplit 
            from urlparse import urlunsplit

            url = urlsplit(options.action)
            
            # if exist query string
            if url[3]:
                query_string = url[3] + "&"+urlencode(fields)
            else:
                query_string = urlencode(fields)
            action = urlunsplit((url[0], url[1], url[2], query_string, url[4]))
            body=None

        else:
            action = options.action
            body=urlencode(fields)

        if options.debug:
            print "Sending request:" 
            print "URL: %s" % action
            print "Method: %s" % options.method
            print "Headers: %s" % headers
            print "Body: %s" % body
            print ""

        resp, content = http.request(action , options.method, body=body, headers=headers)
    
    except Exception, e:
        print "CRITICAL : %s" % e 
        sys.exit(2)
        
    if options.debug:
        print ""
        print "Response: %s" % resp
        print "Content: %s" % content
        print ""
 
    if content:
        if options.content_match:
            try:
                p = re.compile(options.content_match)
            except Exception, e:
                print "Error: regexp not valid: %s" % e
                sys.exit(2)

            m = p.search(content, re.MULTILINE)
            if m:
                print "OK Found: "  + m.group()
                sys.exit(0)
        
    if options.response_match:
        if resp.has_key(response_name):
            try:
                p = re.compile(response_value)
            except Exception, e:
                print "Error: regexp not valid: %s" % e
                sys.exit(2)

            m = p.search(resp[response_name])
            if m:
                print "OK Found: "  + m.group()
                sys.exit(0)

        else:
            print "CRITICAL: response header %s not found" % response_name
            sys.exit(2)
    
    print "CRITICAL : Not found"
    sys.exit(2)




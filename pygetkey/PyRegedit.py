# -*- Mode: Python -*-
# vi:si:et:sw=4:sts=4:ts=4
#
# Copyright (C) 2008 <pietro@bertera.it>

# This file may be distributed and/or modified under the terms of
# the GNU General Public License version 2 as published by
# the Free Software Foundation.
# This file is distributed without any warranty; without even the implied
# warranty of merchantability or fitness for a particular purpose.

import _winreg as wreg
import sys, getopt

class PyRegedit:
    def __init__(self, data_path, root=wreg.HKEY_LOCAL_MACHINE , host=None):
        self.DATA_PATH=data_path
        
        if host:
            self.host = host
        else:        
            self.host = None

        self.registry = wreg.ConnectRegistry(self.host,wreg.HKEY_LOCAL_MACHINE)

    def _readValues(self):
        self._openKeyRO()
        id = 0
        self.values = {}
        while 1:
            try:
                n,v,t = wreg.EnumValue(self.key, id)
                
                self.values[n.lower()] = {"value":v , "type": t}
            
            except EnvironmentError:
                self._closeKey()
                break
            id += 1
        self._closeKey()

    def _readKeys(self):
        self._openKeyRO()
        id = 0
        self.keys = []
        while 1:
            try:
                self.keys.append(wreg.EnumKey(self.key, id).lower())
            except EnvironmentError:
                self._closeKey()
                break
            id += 1
        self._closeKey()

    def _openKeyRO(self):
        self.key = wreg.OpenKey(self.registry, self.DATA_PATH)
        
    def _openKeyRW(self):
        self.key = wreg.OpenKey(self.registry, self.DATA_PATH, 0, wreg.KEY_SET_VALUE)
    
    def _closeKey(self):
        wreg.CloseKey(self.key)
    
    def searchValue(self, value):
        print value
        registry = wreg.ConnectRegistry(self.host,wreg.HKEY_LOCAL_MACHINE)
        self._readValues()
        if value in self.values.keys():
            return True
        else:
            return False
        

    def getValue(self, name):
        self._readValues()
        return  self.values[name]
    
    def listValues(self):
        self._readValues()
        return self.values.keys()
    
    def listKeys(self):
        self._readKeys()
        return self.keys
        
    def setValueNum(self, name, value):
        self._openKeyRW()
        wreg.SetValueEx(self.key, name, 0, wreg.REG_DWORD, int(value))
        self._closeKey()

    def setValueTxt(self, name, value):
        self._openKeyRW()
        wreg.SetValueEx(self.key, name, 0, wreg.REG_SZ, value)
        self._closeKey()
        
    def delValue(self, name):
        self._openKeyRW()
        wreg.DeleteValue(self.key, name)
        self._closeKey()

if __name__ == "__main__":
    r = PyRegedit(r'software\microsoft\active setup')
    r.searchValue("InstallRoot")

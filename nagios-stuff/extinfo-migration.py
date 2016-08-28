#!/usr/bin/python
# vi:si:et:sw=4:sts=4:ts=4
# -*- coding: UTF-8 -*-
# -*- Mode: Python -*-
#
# Copyright (C) 2006 Pietro Bertera <pietro@bertera.it>

# This file may be distributed and/or modified under the terms of
# the GNU General Public License version 2 as published by
# the Free Software Foundation.
# This file is distributed without any warranty; without even the implied
# warranty of merchantability or fitness for a particular purpose.
# See "LICENSE.GPL" in the source distribution for more information.

# This script migrate extinfo data of nagios 1.x into nagios 2.x text format

import MySQLdb
import sys
import os

# Mysql Host
host = "localhost"
# Mysql DB
db = 'nagios'
# Mysql user
user = ''
# Mysql password
passwd = ''

if __name__ == "__main__":
    conn_db = MySQLdb.connect(host=host, user=user, passwd=passwd,db=db)
    cursor_db = conn_db.cursor()
    cursor_db.execute("SELECT * FROM hostextinfo")
    results = cursor_db.fetchall()

    for r in results:
        host_name   = r[0]
        notes_url   = r[1]
        icon_image  = r[2]
        vrml_image  = r[3]
        gd2_icon_image = r[4]
        icon_image_alt = r[5]
        x_2d        = r[6]
        y_2d        = r[7]
        x_3d        = r[8]
        y_3d        = r[9]
        z_3d        = r[10]
        have_2d_coords  = r[11]
        have_3d_coords  = r[12]

        text = "\ndefine hostextinfo {\n"

        if host_name:
            text += "\thost_name\t%s\n" % host_name
        if notes_url:
            text += "\tnotes_url\t%s\n" % notes_url
        if icon_image:
            text += "\ticon_image\t%s\n" % icon_image
        if vrml_image:
            text += "\tvrml_image\t%s\n" % vrml_image
        if gd2_icon_image:
            text += "\tstatusmap_image\t%s\n" % gd2_icon_image
        if icon_image_alt:
            text += "\ticon_image_alt\t%s\n" % icon_image_alt
        if have_2d_coords:
            text += "\t2d_coords\t%s,%s\n" % (x_2d,y_2d)
        if have_3d_coords:
            text += "\t3d_coords\t%s,%s,%s\n" % (x_3d,y_3d,z_3d)

        text +="}\n"

        print text

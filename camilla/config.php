<?php
/*
 *  Camilla: booking system 
 *  Copyright (C) 2005, Bertera Pietro
 *  Authors:
 *  Bertera Pietro
 *  Rolando Rampoldi
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Contacts:
 *  pietro@bertera.it
 */

    $rich_baseDir = '/var/www/camilla';
    $rich_dbDsn = 'mysql://root:pippo@localhost/camilla';
    $rich_persoConfig = 'lib/perso/config.php';
    $rich_adodb = $rich_baseDir.'/lib/adodb';
    $rich_idleTime = 900;
    $rich_template = 'test';
    $rich_url = 'http://www.ciccio.com/camilla';
    $rich_title = 'Camilla Booking';
    $rich_lang = 'it';
    $rich_helpDir = $rich_baseDir.'/help';
    define('_RICH_ADMIN','admin');
    define('_RICH_EXPIRE', '120'); //in sec
    require_once($rich_baseDir.'/functions.php');
?>

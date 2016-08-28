<?php 
/*
 *  PersO: Php Persistent Objects 
 *
 *  Authors:
 *  Bertera Pietro
 *  Copiright (C) 2005, Bertera Pietro
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

define('PERSO_CONF_ARRAY', '_PERSO');

${PERSO_CONF_ARRAY}['DB_DSN'] = $rich_dbDsn;
${PERSO_CONF_ARRAY}['ERROR_FUNC'] = 'persoErrorHandler';
${PERSO_CONF_ARRAY}['ROOT'] = $rich_baseDir.'/lib/perso';
${PERSO_CONF_ARRAY}['ADODB'] = $rich_adodb;
${PERSO_CONF_ARRAY}['LIB'] = ${PERSO_CONF_ARRAY}['ROOT'].'/lib';


function persoErrorHandler($str, $level){
    switch ($level){
        case -1:
            die("FATAL ERROR: $str");
        break;
        case 0:
            //IGNORE
        break;
        
        default:
            echo "ERROR: $str";
    }
}

?>

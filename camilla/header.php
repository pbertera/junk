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

require_once('config.php');
ini_set('include_path',ini_get('include_path').':'.$rich_baseDir.'/lib');
require_once($rich_persoConfig);
require_once('actionMap.php');
if(!defined('_RICH_USERNAME')){
    require_once('auth.php');
}
switch($_REQUEST['action']){
    case ACT_CHANGE_LANGUAGE:
        $languages = getLanguages();
        if(in_array($_REQUEST['task'],$languages)){
            require_once($rich_baseDir.'/languages/'.$_REQUEST['task'].'.php');
            $_SESSION['language'] = $_REQUEST['task'];
        }else{
            displayError("Error in language loading");
        }
    break;
    case ACT_LOGOUT:
        //session_destroy();
        logout();
        header("Location: ?");
        break;
}
if(isset($_GET['yearID'])){
    $_SESSION['yearID'] = $_GET['yearID'];
}
if(isset($_SESSION['yearID'])){
    $_GET['yearID'] = $_SESSION['yearID'];
}
if(!isset($_SESSION['language']) and $_REQUEST['action'] != 'cambia_lingua'){
    require_once($rich_baseDir.'/languages/'.$rich_lang.'.php');
    $_SESSION['language'] = $rich_lang;
}
if(isset($_SESSION['language'])){
    require_once($rich_baseDir.'/languages/'.$_SESSION['language'].'.php');
}
if(isset($_REQUEST['casa'])){
    $_SESSION['casa'] = $_REQUEST['casa'];
}

?>

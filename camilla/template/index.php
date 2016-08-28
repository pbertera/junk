<?php
/*
 *  Camilla: booking system 
 *
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
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <link type="text/css" href="<?php echo $rich_url.'/template/'; ?>/css/style.css" rel="stylesheet" />
    <link type="text/css" href="<?php echo $rich_url.'/template/'; ?>/css/calendar.css" rel="stylesheet" />
    <script src="<?php echo $rich_url?>/lib/js/colorpicker.js" type="text/javascript"></script>
    <script type="text/javascript" src="<?php echo $rich_url ?>/lib/js/richjavascript.js"></script>
    <!-- import the calendar script -->
    <script type="text/javascript" src="<?php echo $rich_url ?>/lib/js/calendar.js"></script>
    <!-- import the language module -->
    <script type="text/javascript" src="<?php echo $rich_url ?>/lib/js/calendar-en.js"></script>
    <title>Camilla Booking</title>
</head>

<body id="body">
    <div id="container">
        <div id="banner">
            <div id="metanavi">
                <?php
                if(isUser()){
                echo _RICH_USERNAME;
                makeLogoutLink();
                }
                ?><br>
                <form action="index.php" method="GET">
                    <?php echo LAN_SELECT_LANGUAGE?>
                    <select name="task">
                <?php
                foreach(getLanguages() as $lan){
                    ?>
                    <option value="<?php echo $lan?>"><?php echo $lan?></option>
                    <?php
                }
                ?>
                    </select>
                    <input type="hidden" name="action" value="<?php echo ACT_CHANGE_LANGUAGE ?>">
                    <input type="submit" value="Go">
                </form>    
            </div>
            <h1>
                <a href="?">CamillaBooking.net</a>
            </h1>
            <h2><?php 
                if(defined('_RICH_CASA'))
                    echo LAN_SUBTITLE._RICH_CASA;
                ?>
            </h2>
        </div>
        <div id="topnavi">
            <?php 
            if(isUser()){
                printMenu();
            }else{
                echo LAN_LOGIN ;
            }
            ?>
            <a href=""><?php echo LAN_CONTACT ?></a>
            <a href="help/cb_user_manual.pdf"><?php echo LAN_MANUAL ?></a>
        </div>
        <?php if(isUser()){?>
        <div id="right">
        <h3><?php echo LAN_HELP ?></h3>
            <?php makeHelp();?>
        </div>
        <?php }?>
        <div id="content">
           <?php 
           if(isUser()){
               require_once('main.php'); 
           }else{
               login();
           }
           ?> 
        </div>
        <div id="footer">Powered by: <a href= "http://thewebfarm.ath.cx">[ thewebfarm ]</a>
    </div>
</body>
</html>

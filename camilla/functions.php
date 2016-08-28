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
    function makeLogoutLink(){
           ?> 
                <a href="<?php echo $rich_url?>?action=<?php echo ACT_LOGOUT?>"><?php echo LAN_LOGOUT?></a>
           <?     
    }
    
    function printMenu(){
        echo LAN_MENU;
        $_REQUEST['action']==ACT_NEW_OPTION?'active':'menu';
        echo $class;
    ?>
       <a class="<?php echo $_REQUEST['action']==ACT_NEW_OPTION?'selected':'menu'?>" href="?action=<?php echo ACT_NEW_OPTION?>"><?php echo LAN_MENU_NEW ?></a>
    <?php 
        if(isAdmin()){
    ?>
            <a class="<?php echo $_REQUEST['action']==ACT_CALENDAR_MANAGEMENT?'selected':'menu'?>" href=?action=<?php echo ACT_CALENDAR_MANAGEMENT?>><?php echo LAN_MENU_CALENDAR ?></a>
            <a class="<?php echo $_REQUEST['action']==ACT_OPTIONS_EXPIRED_MANAGEMENT?'selected':'menu'?>" href=?action=<?php echo ACT_OPTIONS_EXPIRED_MANAGEMENT?>><?php echo LAN_MENU_OPTION ?></a>
            <a class="<?php echo $_REQUEST['action']==ACT_USERS_MANAGER?'selected':'menu'?>" href=?action=<?php echo ACT_USERS_MANAGER?>><?php echo LAN_USERS_MANAGER ?></a>
    <?php
        }
    }
    
    function makeHelp(){
        global $rich_helpDir;
        $act = 'help';
        if(isset($_REQUEST['action'])){
            $act = $_REQUEST['action'];
        }
        $user = 'user';
        if(_RICH_USERNAME == _RICH_ADMIN){
            $user = _RICH_ADMIN;
        }
        $file = $rich_helpDir.'/'.$act."-".$user."-".$_SESSION['language'].".php";
        if(file_exists($file)){
            include_once($file);
        }else {
            echo $file;
        }
        
    }
    
    function getLanguages(){
        global $rich_baseDir;
        if(is_dir($rich_baseDir.'/languages')){
            if($dh = (opendir($rich_baseDir.'/languages'))){
                while(($file = readdir($dh)) != false){
                    if($file != '.' and $file != '..'){
                        $lan[] = $file;
                    }
                }
            }else{
                displayError('Cannot open languages Directory.');
            }
        }else{
            displayError("Language directory is not a directory ?!");           
        }
        foreach($lan as $l){
            if($l != '.' or $l != '..'){
                $language[] = substr($l, 0, -4);
            }
        }
        if(!is_array($language)){
            displayError("No languages files found.");
        }
        return $language;
    }
    function makeColor($i){
        $r = substr((string)dechex($i/500),1,6);
        return $r;
    }
    
    function date2unix($date){
        $d = explode('/',$date);
        $anno = $d[2];
        $mese = $d[1];
        $giorno = $d[0];
        return mktime(0,0,1, $mese, $giorno, $anno);
    }
    function isAdmin(){
        if(_RICH_USERNAME == _RICH_ADMIN){
            return true;
        }
        return false;
    }
    function isUser(){
        if(defined('_RICH_USERNAME')){
            return _RICH_USERNAME;
        }else{
            return false;
        }
    }

    function permissionDenied(){
        echo "Permession Denied.";
        die();
    }

    function makeDatePicker($name){
        $id = $name.rand(0,255);
        ?>
        <input type="text" name="<?php echo $name?>" id="<?php echo $id?>" size="12" maxlength="10" readonly>
        <input value="..." onclick="return showCalendar('<?php echo $id?>','dd-mm-y');" type="reset">
        <?php
    }

    function makeColorPicker($name){
        $id=$name.rand(0,255);
        ?>
        <input id="<?php echo $id?>field" readonly size="7" onChange="cp.relateColor(this.value);" title="onclick"
            name="<?php echo $name ?>"><a href="javascript:void(0)" onclick="cp.pickColor();" id="<?php echo $id?>" style="border: 1px solid #000000"
        >&nbsp;&nbsp;&nbsp;</a>
        <script language="javascript">
            var cp = new ColorPicker( 'cp', '<?php echo $id?>', '#ffffff' );
        </script>

        <?php
    } 

    function displayError($text){
        ?>
        <div id="error">
            <?php 
                echo $text;
            ?>
        </div>
        <?php 
    }
?>

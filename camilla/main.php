<?
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

    require_once('objects/Opzione.php');
    require_once('objects/OpzioneScaduta.php');
    require_once('objects/Calendario.php');
    require_once('actionMap.php');
    //TODO: security check!!
    if(!isUser()){
        login();
    }
    purgeOpzioni();
    switch($_REQUEST['action']){
       case ACT_NEW_OPTION:
            switch($_REQUEST['task']){
                case ACT_NEW_OPTION_SUBMIT:
                    creaOpzione($_REQUEST[ACT_NEW_OPTION]);
                break;
                default: 
                    nuovaOpzioneForm();
                break;
            }
            break;
        case ACT_VIEW_OPTION:
            mostraOpzione($_REQUEST['opzione']);
        break;
        case ACT_MODIFY_OPTION:
            switch($_REQUEST['task']){
                case LAN_DELETE:
                    if(isAdmin()){
                        eliminaOpzione($_REQUEST['id']);
                    }else{
                        permissionDenied();
                    }
                break;
                case LAN_CONFIRM:
                    if(isAdmin()){
                        confermaOpzione($_REQUEST['id']);
                    }else{
                        permissionDenied();
                    }
               break;
            }
        break;    
        case ACT_CALENDAR_MANAGEMENT:
            if(isAdmin()){
                switch($_REQUEST['task']){
                    case ACT_CALENDAR_MANAGEMENT_ADD:
                        //TODO: validare input
                        $inizio = date2unix($_REQUEST['inizio']);
                        $fine = date2unix($_REQUEST['fine']);
                        $color = $_REQUEST['colore'];
                        $c =& new Calendario($inizio,$fine,$color);
                        $c->store(md5($inizio.$fine));
                    break;
                    case ACT_CALENDAR_MANAGEMENT_DELETE:
                        $c =& new Calendario('','','');
                        //TODO: validare input
                        $id = $_REQUEST['id'];
                        $c->delete($id);
                    default:
                        modificaCalendarioForm();
                    break;
                }
            }else{
                permissionDenied();
            }
        break;
        case ACT_OPTIONS_EXPIRED_MANAGEMENT:
            switch($_REQUEST['task']){
                case ACT_OPTIONS_EXPIRED_MANAGEMENT_DELETE:
                    if(isAdmin()){
                        //TODO: validare input
                        $id = $_REQUEST['id'];
                        eliminaOpzioneScaduta($id);
                    }
                default:
                    if(isAdmin()){
                        showOpzioniScadute();
                    }
            }
        break;
        case ACT_USERS_MANAGER: 

            if(isAdmin()){
                switch($_REQUEST['task']){
                    case ACT_USERS_MANAGER_DELETE:
                        deleteUser($_REQUEST['id']);
                    break;
                    case ACT_USERS_MANAGER_ADD:
                        addUser($_REQUEST[ACT_ADD_USER]['username'],$_REQUEST[ACT_ADD_USER]['password'],$_REQUEST[ACT_ADD_USER]['password1'],$_REQUEST[ACT_ADD_USER]['email']);
                    break;
                    case ACT_USERS_MANAGER_MODIFY:
                        $u = $_REQUEST['id'];
                    break;
                    case ACT_USERS_MANAGER_MODIFY_SUBMIT:
                        modifyUser($_REQUEST[ACT_MODIFY_USER]['username'],$_REQUEST[ACT_MODIFY_USER]['password'],$_REQUEST[ACT_MODIFY_USER]['password1'],$_REQUEST[ACT_MODIFY_USER]['email']);
                    break;
                }
                showUsersList($u);
            }
        break;
    }
    mostraCalendario();
    
    function modifyUser($username, $pwd, $pwd1, $email){
        if(isset($pwd)){
            if($pwd != $pwd1){
                displayError(LAN_PASSWORD_NOT_MATCH);
            return;
            }
        }
        $u =& new User();
        $u =& $u->fetch($username);
        $u->username = $username;
        $u->email = $email;
        if($pwd != ''){
            if($pwd != $pwd1){
                displayError(LAN_PASSWORD_NOT_MATCH);
                return;
            }else{
                $u->password = md5($pwd);
            }
        }
        $u->store($username);
    }
    
    function addUser($username, $pwd, $pwd1, $email){
        if($pwd != $pwd1){
            displayError(LAN_PASSWORD_NOT_MATCH);
            return;
        }
        $u =& new User($username, $pwd, $email);
        $u->store($username);
    }
    
    function deleteUser ($id){
       $user =& new User();
       $user->delete($id);
    }
    
    function showUsersList($user=''){
        $us =& new User();
        ?>
        <div id="area">
        <?php
        if($users = $us->fetchAll()){
            foreach($users as $u){
                ?>
                <div id="subarea">
                <b>
                <?php echo LAN_USER_NAME?>: </b><?php echo $u->username ?><b> <?php echo LAN_USER_EMAIL?>: </b><?php echo $u->email?>
                <?php
                $text = LAN_DEL_USER_WARNING.$u->username.' ?';
                echo "  <span id=\"elimina\"><a href=\"?action=".ACT_USERS_MANAGER."&task=".ACT_USERS_MANAGER_DELETE."&id=$u->username\"
                onClick=\"javascript:return confirm('$text');\">".LAN_DELETE."</a></span>";
                echo "<span id=\"modifica\"><a href=\"?action=".ACT_USERS_MANAGER."&task=".ACT_USERS_MANAGER_MODIFY."&id=$u->username\">".LAN_MODIFY."</a></span>";
                ?>
                </div>
                <?php
            }
        } else {
            echo LAN_NO_USERS;
        }
        ?>
        </div>
        <div id="area">
        <?php 
        if($user == ''){
        ?>
        <span id="title"><?php echo LAN_ADD_USER?>:</span><br>
        <form action="?action=<?php echo ACT_USERS_MANAGER?>&task=<?php echo ACT_USERS_MANAGER_ADD?>" method="POST">
            <?php echo LAN_USER_NAME?>: <input type="text" name="<?php echo ACT_ADD_USER?>[username]"><br>
            <?php echo LAN_USER_EMAIL?>: <input type="text" name="<?php echo ACT_ADD_USER?>[email]]"><br>
            <?php echo LAN_USER_PASSWORD?>: <input type="password" name="<?php echo ACT_ADD_USER?>[password]"><br>
            <?php echo LAN_USER_PASSWORD_CONFIRM?>: <input type="password" name="<?php echo ACT_ADD_USER?>[password1]"><br>
            <input type="submit" name="add" value="<?php echo LAN_ADD?>">
        </form>
        <?php 
        } else {
            $u =& $us->fetch($user);
            $username = $u->username;
            $email = $u->email;
            ?>
            <span id="title"><?php echo LAN_MODIFY_USER?>:</span><br>
            <form action="?action=<?php echo ACT_USERS_MANAGER?>&task=<?php echo ACT_USERS_MANAGER_MODIFY_SUBMIT?>" method="POST">
            <?php echo LAN_USER_NAME?>: <input type="text" name="<?php echo ACT_MODIFY_USER?>[username]" value="<?php echo $username?>"><br>
            <?php echo LAN_USER_EMAIL?>: <input type="text" name="<?php echo ACT_MODIFY_USER?>[email]]" value="<?php echo $email?>"><br>
            <?php echo LAN_USER_PASSWORD?>: <input type="password" name="<?php echo ACT_MODIFY_USER?>[password]"><br>
            <?php echo LAN_USER_PASSWORD_CONFIRM?>: <input type="password" name="<?php echo ACT_MODIFY_USER?>[password1]"><br>
            <input type="submit" name="add" value="<?php echo LAN_MODIFY?>">
        </form>
     
            <?php 
        }
        ?>
        </div>
        <?php
    }
    
    function showOpzioniScadute(){
        $o =& new OpzioneScaduta('','','');
        ?>
        <div id="area">
        <?php
        if($op = $o->fetchAll()){
            foreach($op as $o){
                $id = md5($o->dataInizio.$o->dataFine.$o->cliente.$o->user.$o->casa);
                ?>
                <div id="subarea">
                <?php
                echo "<b>".LAN_DATE_START.":</b> ".date("d/m/Y",$o->dataInizio);
                echo "<b> ".LAN_DATE_END.":</b> ".date("d/m/Y",$o->dataFine) ;
                echo "<b> ".LAN_AGENCY.":</b> $o->user ";
                echo "<b> ".LAN_HOUSE.":</b> $o->casa ";
                echo "<b> ".LAN_CUSTOMER.": </b>$o->cliente ";
                $text = LAN_WARN_DELETE_OPTION_EXPIRED."$o->user ?";
                $yesLink =  "?action=".ACT_OPTIONS_EXPIRED_MANAGEMENT."&task=".ACT_OPTIONS_EXPIRED_MANAGEMENT_DELETE."&id=$id";
                $noLink =  "?action=".ACT_OPTIONS_EXPIRED_MANAGEMENT;
                
                echo "<span id=\"elimina\"> 
                <a href=\"?action=".ACT_OPTIONS_EXPIRED_MANAGEMENT."&task=".ACT_OPTIONS_EXPIRED_MANAGEMENT_DELETE."&id=$id\" onclick=\"javascript:return confirm('$text')\">
                    ".LAN_DELETE."
                </a>
                <br></span>";
                ?>
                </div>
                <?php
            }
        }else {
            echo LAN_NO_EXPIRED_OPTIONS;
        }
        ?>
        </div>
        <?php
    }

    function eliminaOpzioneScaduta($id){
        $o =& new OpzioneScaduta('','','');
        $o->delete($id);
    }
    
    function nuovaOpzioneForm(){
        $text = LAN_WARN_NEW_OPTION;
        ?>
        <div id="area">
        <form action="?action=<?php echo ACT_NEW_OPTION?>&task=<?php echo ACT_NEW_OPTION_SUBMIT?>" method="POST">
            <?php echo LAN_CUSTOMER?>: <input type="text" name="<?php echo ACT_NEW_OPTION?>[cliente]"><br>
            <?php echo LAN_DATE_START?>: <?php makeDatePicker(ACT_NEW_OPTION."[data_inizio]")?><br>
            <?php echo LAN_DATE_END?>: <?php makeDatePicker(ACT_NEW_OPTION."[data_fine]")?><br>
            <input type="submit" name="opziona" value="Opziona" onClick="javascript:return confirm('<?php echo $text ?>');">
        </form>
        </div>
        <?php
    }

    function modificaCalendarioForm(){
        ?>
        <div id="area">
        <?php 
        $calendario =& new Calendario('','','');
        if($cals = $calendario->fetchAll()){
            $i = 0;
            foreach($cals as $ca){
                ?>
                <div id="subarea">
                <b><?php echo LAN_SEASON." :" ?> </b><?php echo $i?> 
                <b><?php echo LAN_DATE_START." :"?></b><?php echo date('d/m/Y',$ca->inizio)?>
                <b><?php echo LAN_DATE_END." :"?></b><?php echo date('d/m/Y',$ca->fine)?>
                <b><?php echo LAN_COLOR." :"?></b><span id="color" style="background:<?php echo $ca->colore?>">&nbsp;&nbsp;&nbsp;</span>
                <span id="elimina">
                <a href=?action=<?php echo ACT_CALENDAR_MANAGEMENT?>&task=<?php echo ACT_CALENDAR_MANAGEMENT_DELETE?>&id=<?php 
                echo md5($ca->inizio.$ca->fine)?>><?php echo LAN_DELETE?></a>
                </span>
                </div>
                <?
                $i++;
            }
        }else{
            echo LAN_NO_SEASON;
        }
        ?>
        <span id="title"><?php echo LAN_ADD_SEASON?>:</span><br>
        <form action="?action=<?php echo ACT_CALENDAR_MANAGEMENT?>&task=<?php echo ACT_CALENDAR_MANAGEMENT_ADD?>" method="POST">
            <?php echo LAN_DATE_START ?> :<?php makeDatePicker("inizio");?><br>
            <?php echo LAN_DATE_END ?> :<?php makeDatePicker("fine");?><br>
            <?php echo LAN_COLOR ?> :<?php makeColorPicker("colore")?> &lt;- <?php echo LAN_CLICK?> <br>
            <input type="submit" name="add" value="<?php echo LAN_ADD?>">
        </form>
        </div>
        <?php
    }

    function creaOpzione($opzione){
        //TODO: validare Input
        if(date2unix($opzione['data_fine']) < date2unix($opzione['data_inizio'])){
            displayError(LAN_ERR_DATE_NEW_OPTION);
            return;
        }
        $op = new OpzioneScaduta("");
        if($opS = $op->fetch(md5(date2unix($opzione['data_inizio']).date2unix($opzione['data_fine']).$opzione['cliente']._RICH_USERNAME._RICH_CASA))){
             displayError(LAN_ERR_OPTION_EXPIRED);
             return;
        }
        $o =& new Opzione(date2unix($opzione['data_inizio']), 
            date2unix($opzione['data_fine']),
            $opzione['cliente']);
        if($opzioni = $o->fetchAll()){
            foreach($opzioni as $op){
                if($op->stato == 1){
                    $dataI = $op->dataInizio;
                    $dataF = $op->dataFine;
                    $dataNuovaI = $o->dataInizio;
                    $dataNuovaF = $o->dataFine;
                    if( ($dataNuovaI < $dataF && $dataNuovaI >= $dataI) || 
                        ($dataNuovaF > $dataI && $dataNuovaF <= $dataF) ||
                        ($dataNuovaI <= $dataI && $dataNuovaF >= $dataF)){
                        $sovrapp = true;
                    }
                }
            }
        }
        if(!$sovrapp){
            $o->store(md5($o->dataInizio.$o->dataFine.$o->user.$o->casa));
        }else{
            displayError(LAN_RESERVATION_FROM.date("d/m/Y",$dataNuovaI)." ".LAN_RESERVATION_TO." ".date("d/m/Y", $dataNuovaF)." ".LAN_RESERVATION_CONFLICT);
        }
    }

    function mostraCalendario(){
        $o =& new Opzione('a','b','c');
        $opzioni = $o->fetchAll();
        include('lib/activecalendar/activecalendar.php');
        $cal = new activeCalendar($_GET['yearID']);
        $cal->enableYearNav();
        $cal->enableMonthNav();
        $j = 0;
        if($opzioni)
        foreach($opzioni as $op){
            if($op->casa == _RICH_CASA){
                $timeI = $op->dataInizio;
                $timeF = $op->dataFine;
                $r = makeColor($j);
                $g = makeColor($j);
                $b = makeColor($j);
                for($a=$timeI;$a<$timeF+86400;$a+=86400){
                    $color = makeColor($timeI.$timeF);
                    $content = '<div style="margin: 0px; padding: 0px; width: 10px; height: 2px; background:#'.$color.';"</div>';
                    $cal->setEventContent(date('Y',$a),
                                            date('m',$a),
                                            date('d',$a),
                                            $content,
                                            '?action='.ACT_VIEW_OPTION.'&opzione='.md5($op->dataInizio.$op->dataFine.$op->user.$op->casa),
                                            $op->stato?'prenotazione':'opzione');  
                    
                } 
                $j++;
            }
        }
        $stagione =& new Calendario('','','');
        $st = $stagione->fetchAll();
        if($st)
        foreach($st as $s){
            $stagioni[] = array('inizio' => $s->inizio, 'fine' => $s->fine, 'colore' => $s->colore);
        }
       
        if($stagioni)
        foreach ($stagioni as $s){
                $inizio = $s['inizio'];
                $fine = $s['fine'];
                for ($a=$inizio; $a<$fine+86400; $a += 86400){
                    $giorno = date('d',$a);
                    $mese = date('m',$a);
                    $anno = date('Y',$a);
                    $cal->setEvent($anno, $mese, $giorno, '" style="background: '.$s['colore'].'"');
                }
        }
        print($cal->showYear());
    }

   function mostraOpzione($id){
        ?>
        <div id="area">
        <?php
        $o =& new Opzione(0,0,0);
        $o = $o->fetch($id);
        if(isAdmin()){
            echo "<b>".LAN_USER.": </b>".$o->user."<br>";
            echo "<b>".LAN_CUSTOMER.": </b>".$o->cliente."<br>";
        }
        echo "<b>".LAN_DATE_START.": </b>".date('d/m/Y',$o->dataInizio)."<br>";
        echo "<b>".LAN_DATE_END.": </b>".date('d/m/Y',$o->dataFine)."<br>";
        echo "<b>".LAN_DATE_CREATION.": </b>".date("d/m/Y H:i",$o->time)."<br>";
        if($o->stato == 0)
        echo "<b>".LAN_DATE_EXPIRE.": </b>".date("d/m/Y H:i",$o->time+_RICH_EXPIRE)."<br>";
        echo "<b>".LAN_HOUSE.": </b>".$o->casa."<br>";
        $stato = $o->stato?LAN_STATE_CONFIRMED:LAN_STATE_NOT_CONFIRMED;
        echo "<b>".LAN_STATE.": <b/>".$stato."<br>";
        if(isAdmin()){
            ?>
            <div id="subarea">
            <form action="?" method='GET'>
                <input type="hidden" name="id" value="<?php echo $id?>">
                <input type="hidden" name="action" value="<?php echo ACT_MODIFY_OPTION?>">
                <input type="submit" name="task" value="<?php echo LAN_DELETE?>">
                <?php 
                if($o->stato == 0){
                    ?>
                    <input type="submit" name="task" value="<?php echo LAN_CONFIRM?>">
                    <?php
                }
                ?>
            </form>
            </div>
            </div>
            <?php
        }
    }
    
    function confermaOpzione($id){
       $o =& new Opzione('','','');
       if($o=$o->fetch($id)){
        $o->conferma();
        $o->store($id);
       }
    }

    function eliminaOpzione($id){
       $o =& new Opzione('','','');
       if($o=$o->fetch($id)){
           $o->delete($id);
       }

    }
    
    function purgeOpzioni(){
        $o =& new Opzione('','','');
        if($op = $o->fetchAll()){
            foreach($op as $opz){
                if($opz->stato == 0)
                if((mktime()- $opz->time) > _RICH_EXPIRE){
                    $opScaduta =& new OpzioneScaduta($opz);
                    $opScaduta->store(md5($opz->dataInizio.$opz->dataFine.$opz->cliente.$opz->user.$opz->casa));
                    $opz->delete(md5($opz->dataInizio.$opz->dataFine.$opz->user.$opz->casa));
                }
            }
        }
    }
    
?>

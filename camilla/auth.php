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

include_once('objects/User.php');
require_once('lib/Auth/Auth.php');

class MyAuth extends Auth {
    var $user;
    
    function MyAuth($session = 'auth',$seed = '1234',$idle = 0){
        $this->user =& new User("ss","ss");
        parent::Auth($this, $session, $seed, $idle);
    }
    
    function fetchData($username, $password){
        $this->user = $this->user->fetch($username);
        if($this->user->username == $username && 
            md5($password) == $this->user->password){
            $_SESSION['casa'] = $_POST['casa'];
            return true;
        }
        return false;
    }
}

function login (){
    ?>
    <div id="login">
    <form method="POST" action="<?php echo $_SERVER['PHP_SELF']?>">
        <?php echo LAN_USER ?> : <input type="text" name="username" class="username"><br>
        <?php echo LAN_PASSWORD?> : <input type="password" name="password" class="password"><br>
        <?php echo LAN_HOUSE?> : 
        <select name="casa">
            <option value="villa_camilla">Villa Camilla</option>
            <option value="villa_elsa">Villa Elsa</option>
        </select><br>
        <input type="submit" value="LogIn">
    </form>
    </div>
    <?php
}

if(!defined(_RICH_USERNAME)){
    
$auth = new MyAuth($rich_authSession, $rich_seed, $rich_idleTime);

if($auth->checkAuth()){
    define('_RICH_USERNAME',$auth->getUsername());
    define('_RICH_CASA',$_SESSION['casa']);
}
}

function logout(){
    global $auth;
    $auth->logout();
    define('_RICH_USERNAME',false);
    
}
?>

<?php
/*
 *  Camilla: booking system 
 *  Copyright (C) 2005, Bertera Pietro)
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


class Auth {
   var $session;
   var $seed;
   var $idle;
      
   function Auth($container, $session = 'auth',$seed = '1234',$idle=0){
        session_start();
        $this->session = $session;
        $this->seed = $seed;
        $this->idle = $idle;
        
        if(!isset($_SESSION[$this->session])){
            if($container->fetchData($_REQUEST['username'], $_REQUEST['password'])){
                $_SESSION[$session]['username'] = $_REQUEST['username'];
                $_SESSION[$session]['seed'] = md5($_REQUEST['username'].$seed);
                if($idle != 0){
                    $_SESSION[$session]['time'] = mktime();
                }
            } else {
                return false;
            }
        }
   }
   
   function checkAuth(){
        if($this->_checkSession())
            return true;
        else
            return false;        
   }
   
   function getUsername(){
       return $_SESSION[$this->session]['username'];
   }
   
   function logout(){
       if(isset($_SESSION[$this->session]['username']) and isset($_SESSION[$this->session]['seed'])){
            unset($_SESSION[$this->session]);
            return true;
       }
       return false;
   }

   function _checkSession(){
       if(isset($_SESSION[$this->session]['username']) and isset($_SESSION[$this->session]['seed'])){
           if(md5($_SESSION[$this->session]['username'].$this->seed) == $_SESSION[$this->session]['seed']){
               if($this->idle != 0){
                   $time = mktime();
                   if(($time - $_SESSION[$this->session]['time']) > $this->idle){
                       $this->logout();
                       return false;
                   }
                   $_SESSION[$this->session]['time'] = mktime();
               }
               return true;
           } else {
               $this->logout();
               return false;
           }
       }
       $this->logout();
       return false;
   }
}
?>

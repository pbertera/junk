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

include ${PERSO_CONF_ARRAY}['ADODB']."/adodb.inc.php";

class PersOObjectDb {
    
    function PersOObjectDb($name){
        global ${PERSO_CONF_ARRAY};
        if(!(is_object(${PERSO_CONF_ARRAY}['DB_LINK']) 
            or method_exists(${PERSO_CONF_ARRAY}['DB_LINK'],'isConnected')
            and ${PERSO_CONF_ARRAY}['DB_LINK']->IsConnected())){
            // non connesso
            ${PERSO_CONF_ARRAY}['DB_LINK'] = @ADONewConnection(${PERSO_CONF_ARRAY}['DB_DSN']);
            if(!${PERSO_CONF_ARRAY}['DB_LINK']){
                // crea connessione
                ${PERSO_CONF_ARRAY}['ERROR_FUNC'](DB_CONN_ERR,-1);
            }
            ${PERSO_CONF_ARRAY}['DB_LINK']->setFetchMode(ADODB_FETCH_ASSOC);
        }
        $res = &${PERSO_CONF_ARRAY}['DB_LINK']->MetaTables();
        if(!$res){
            ${PERSO_CONF_ARRAY}['ERROR_FUNC'](SHOW_OBJECTS_ERROR.': '.${PERSO_CONF_ARRAY}['DB_LINK']->ErrorMsg(), 0);
        } 
        if(!in_array($name, $res)){
            $query = 'CREATE TABLE `'.$name.'` (id char(32) primary key, object text)';
            $this->_query($query, 'ERROR_CREATE_STORE', -1);
        }
    }

    function store($id, &$obj,$name){
        global ${PERSO_CONF_ARRAY};
        $o = ${PERSO_CONF_ARRAY}['DB_LINK']->qstr(serialize($obj));
        $query = 'INSERT INTO `'.$name.'` (id, object) VALUES (\''.$id.'\','.$o.')';
        if(!$this->_query($query,STORE_INSERT_ERROR, 0)){
            $query = 'UPDATE `'.$name.'` SET object = '.$o.' WHERE id=\''.$id."'";
            if(!$this->_query($query, STORE_UPDATE_ERROR, -1)){
                return false;
            }
            return true;
        }
        return true;
    }

    function &fetch ($id, $name){
        global ${PERSO_CONF_ARRAY};
        $query = 'SELECT object from `'.$name.'` where id = \''.$id."'";
        if($ret = $this->_query($query,FETCH_OBJ_ERROR,-1)){
            return unserialize($ret[0]['object']);
        }
        return false;
    }

    function delete($id, $name){
        $query = 'DELETE FROM `'.$name.'` WHERE id = \''.$id."'";
        if(!$this->_query($query, DELETE_ERROR, -1)){
            return false;
        }
        return true;
    }
    
    function &fetchAll($name){
       $query = 'SELECT * FROM `'.$name.'`';
       $r = false;
       if($ret = $this->_query($query, FETCH_ALL_ERROR, 0)){
           foreach ($ret as $b ){
            $r[] = unserialize($b['object']);
           }
       }
       return $r;
    }

    function _query($query, $error='', $errLevel=0){
       global ${PERSO_CONF_ARRAY};
       if(!$res = ${PERSO_CONF_ARRAY}['DB_LINK']->Execute($query)){
           ${PERSO_CONF_ARRAY}['ERROR_FUNC'](QUERY_ERROR.': '.$error.' -- '.${PERSO_CONF_ARRAY}['DB_LINK']->errorMsg(), $errLevel);
       }
       if(is_object($res) and $res->RecordCount()>0){
            while(!$res->EOF){
               $r[]=$res->fields;
               $res->MoveNext();
            }
       }
       return $r;
    }
}
?>

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

require_once ${PERSO_CONF_ARRAY}['ROOT']."/PersoObjectDb.php";

class PersO {
    var $_db;
    
    function PersO(){
        $this->_init();
    }

    function store($id=''){
        $this->_init();
        if(!$id)
            $id = get_class($this);
        return $this->_db->store($id,$this,strtolower(get_class($this)));
    }

    function &fetch($id=''){
        $this->_init();
        if(!$id){
            $id = strtolower(get_class($this));
        }
        return $this->_db->fetch($id,strtolower(get_class($this)));
    }

    function delete($id=''){
        $this->_init();
        if(!$id){
            $id = strtolower(get_class($this));
        }
        return $this->_db->delete($id,strtolower(get_class($this)));
    }

    function &fetchAll(){
        $this->_init();
        return $this->_db->fetchAll(strtolower(get_class($this)));
    }

    function _init(){
        if(!is_object($this->_db))
            $this->_db = new PersOObjectDb(strtolower(get_class($this)));
    }
}
?>

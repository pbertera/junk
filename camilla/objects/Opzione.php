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

class Opzione extends Perso{
    var $dataInizio;
    var $dataFine;
    var $cliente;
    var $time;
    var $user;
    var $casa;
    var $stato;
    
    function Opzione ($dataInizio, $dataFine, $cliente){
            $this->time = time();
            $this->dataInizio = $dataInizio;
            $this->dataFine = $dataFine;
            $this->cliente = $cliente;
            $this->user = _RICH_USERNAME;
            $this->casa = _RICH_CASA;
            $this->stato = 0;
    }

    function conferma(){
        $this->stato = 1;
    }
}
?>

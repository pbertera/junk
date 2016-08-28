<?php 
include_once "lib/perso/Perso.php";

class Casa extends Perso {
    var $id;
    var $nome;

    function Casa($id, $nome){
        $this->id = $id;
        $this->nome = $nome;
    }

    function stamp(){
        echo "ID: $this->id -- NOME: $this->nome\n";
    }
}
?>

--TEST--
PHPAst export
--SKIPIF--
<?php if (!extension_loaded("phpast")) print "skip"; ?>
--FILE--
<?php 

function hook($ast) {
    var_dump($ast->export());
}

PHPAst::enableAstHook('hook');

eval('  $a  =  1  ; ');

--EXPECT--
string(8) "$a = 1;
"

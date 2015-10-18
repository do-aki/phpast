--TEST--
PHPAst class exists
--SKIPIF--
<?php if (!extension_loaded("phpast")) print "skip"; ?>
--FILE--
<?php 
var_dump(class_exists('PHPAst'));
--EXPECT--
bool(true)

--TEST--
PHPAst::compileString (failed)
--SKIPIF--
<?php if (!extension_loaded("phpast")) print "skip"; ?>
--FILE--
<?php
var_dump(PHPAst::compileString('i n v a l i d  c o d e'));
--EXPECTF--
Parse error: syntax error, unexpected 'n' (T_STRING) %s

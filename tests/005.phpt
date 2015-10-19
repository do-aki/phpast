--TEST--
PHPAst::compileFile (failed)
--SKIPIF--
<?php if (!extension_loaded("phpast")) print "skip"; ?>
--FILE--
<?php 
PHPAst::compileFile('');
--EXPECTF--
Warning: PHPAst::compileFile(): Filename cannot be empty in %s

Warning: PHPAst::compileFile(): Failed opening '' for inclusion %s

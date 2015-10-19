--TEST--
PHPAst::compileString
--SKIPIF--
<?php if (!extension_loaded("phpast")) print "skip"; ?>
--FILE--
<?php
$ast = PHPAst::compileString('echo 1;');
if ($ast) {
    var_dump($ast->getKindName());
}
--EXPECT--
string(13) "AST_STMT_LIST"

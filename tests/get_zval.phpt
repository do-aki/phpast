--TEST--
isZval/getZval/setZval
--SKIPIF--
--FILE--
<?php

$ast = PHPAst::compileString('0;');

$ast->eachChild(function($ast) {
    var_dump($ast->isZval());
    var_dump($ast->getZval());
    $ast->setZval("1");
    var_dump($ast->getZval());
});

--EXPECTF--
bool(true)
int(0)
string(1) "1"


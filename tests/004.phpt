--TEST--
PHPAst::compileFile
--SKIPIF--
<?php if (!extension_loaded("phpast")) print "skip"; ?>
--FILE--
<?php 
function echoKind($ast, $indent) {
    echo str_repeat(' ', $indent);
    echo $ast->getKindName(), "(", $ast->getChildCount(), ")\n";
    $ast->eachChild(
        function ($child_ast) use ($indent) {
            echoKind($child_ast, $indent+1);
        }
    );
}

echoKind(PHPAst::compileFile(__DIR__ . '/sample.php'), 0);
--EXPECT--
AST_STMT_LIST(2)
 AST_ASSIGN(2)
  AST_VAR(1)
   AST_ZVAL(0)
  AST_ZVAL(0)
 AST_FOR(4)
  AST_EXPR_LIST(1)
   AST_ASSIGN(2)
    AST_VAR(1)
     AST_ZVAL(0)
    AST_ZVAL(0)
  AST_EXPR_LIST(1)
   AST_GREATER(2)
    AST_VAR(1)
     AST_ZVAL(0)
    AST_ZVAL(0)
  AST_EXPR_LIST(1)
   AST_PRE_INC(1)
    AST_VAR(1)
     AST_ZVAL(0)
  AST_STMT_LIST(1)
   AST_ASSIGN_OP(2)
    AST_VAR(1)
     AST_ZVAL(0)
    AST_VAR(1)
     AST_ZVAL(0)


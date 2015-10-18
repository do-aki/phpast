<?php
(new class {

    private $storage;
    
    function __construct() {
        $this->storage = new SplObjectStorage();
    }

    function getId(PHPAst $ast) {
        if (!isset($this->storage[$ast])) {
            $this->storage[$ast] = count($this->storage) + 1;
            echo "{$this->storage[$ast]} [label = \"{$ast->getKindName()}\"]\n";
        }
        return $this->storage[$ast];
    }

    function printNode(PHPAst $ast) {
        $id = $this->getId($ast);
        $ast->eachChild(
            function (PHPAst $child) use ($id, $storage) {
                $child_id = $this->getId($child);
                echo "{$id} -> {$child_id};\n";
                $this->printNode($child);
            }
        );
    }

    function __invoke($ast) {
        echo "digraph ast {\n";
        $this->printNode($ast);
        echo "}\n";
    }
})(PHPAst::compileFile($argv[1]));


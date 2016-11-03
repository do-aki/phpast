<?php
$converter = (new class {

    private $storage;
    private $unique_count = 0;
    private $output = '';

    
    function __construct() {
        $this->storage = new SplObjectStorage();
    }


    function getUniqueId($label) {
        $id = "uq" . ++$this->unique_count;
        $this->output .= "{$id} [label = {$label}]\n";
        return $id;

    }

    function getId(PHPAst $ast) {
        if (!isset($this->storage[$ast])) {
            $this->storage[$ast] = count($this->storage) + 1;

            $label = $ast->getKindName();
            if ($ast->isZval()) {
                $label .= "\\n" . var_export($ast->getZval(), true);
            } elseif (($string = $ast->__toString()) !== '') {
                $label .= "\\n[" . $string . "]";
            }

            $this->output .= "{$this->storage[$ast]} [label = \"{$label}\"]\n";
        }
        return $this->storage[$ast];
    }

    function printNode(PHPAst $ast) {
        $id = $this->getId($ast);
        $ast->eachChild(
            function (PHPAst $child=null) use ($id, $storage) {
                if ($child) {
                    $child_id = $this->getId($child);
                    $this->output .= "{$id} -> {$child_id};\n";
                    $this->printNode($child);
                } else {
                    $null_id = $this->getUniqueId("NULL");
                    $this->output .= "{$id} -> {$null_id};\n";
                }
            }
        );
    }

    function parseCode($code) {
        return $this->__invoke(PHPAst::compileString($code));
    }

    function hasOutput() {
        return (bool)$this->output;
    }

    function output() {
        echo htmlspecialchars($this->output);
    }

    function outputPng() {
        $t_dot = tempnam(sys_get_temp_dir(), 'ast');
        $t_png = tempnam(sys_get_temp_dir(), 'ast');
        file_put_contents($t_dot, $this->output);
        system("dot -Tpng {$t_dot} -o {$t_png}");
        $png = file_get_contents($t_png);
        unlink($t_dot);
        unlink($t_png);
        
        echo "data:image/png;base64," . base64_encode($png);
    }


    function __invoke($ast) {
        static $guard = false;
        if ($guard) {
            return;
        }
        $guard = true;

        $this->output = "digraph ast {\n";
        $this->output .= $this->printNode($ast);
        $this->output .= "}\n";
        return $this->output;
    }
});

if (isset($_POST['code'])) {
    $code = $_POST['code'];
    $converter->parseCode($code);
} else {
    $code = <<<'_EOC_'
function hello($name) {
    echo "Hello {$name}" . PHP_EOL;
}

hello('php');

_EOC_;
}


?>
<html>
<form method="post">
  <textarea name="code" style="width:300px;height:100px;"><?php echo htmlspecialchars($code); ?></textarea>
  <input type="submit" />
</form>


<?php if ($converter->hasOutput()) { ?>
<img src="<?php $converter->outputPng(); ?>">

<pre>
<?php $converter->output(); ?>
</pre>

<?php } ?>

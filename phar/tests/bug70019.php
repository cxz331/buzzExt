<?php
$dir = __DIR__."/bug70019";
$phar = new PharData(__DIR__."/bug70019.zip");
if(!is_dir($dir)) {
  mkdir($dir);
}
$phar->extractTo($dir); 
var_dump(file_exists("$dir/ThisIsATestFile.txt"));
?>
===DONE===

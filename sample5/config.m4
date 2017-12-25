
PHP_ARG_ENABLE(sample5,  
  [Whether to enable the "sample5" extension],  
  [  enable-sample5       Enable "sample5" extension support])  
  
if test $PHP_SAMPLE5 != "no"; then  
  PHP_SUBST(SAMPLE5_SHARED_LIBADD)  
  PHP_NEW_EXTENSION(sample5, sample5.c, $ext_shared)  
fi  

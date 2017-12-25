PHP_ARG_ENABLE(sample4,  
  [Whether to enable the "sample4" extension],  
  [  enable-sample4       Enable "sample4" extension support])  
  
if test $PHP_SAMPLE4 != "no"; then  
  PHP_SUBST(SAMPLE4_SHARED_LIBADD)  
  PHP_NEW_EXTENSION(sample4, sample4.c, $ext_shared)  
fi  

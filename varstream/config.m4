PHP_ARG_ENABLE(varstream,whether to enable varstream support,  
[  enable-varstream      Enable varstream support])  
  
if test "$PHP_VARSTREAM" = "yes"; then  
  AC_DEFINE(HAVE_VARSTREAM,1,[Whether you want varstream])  
  PHP_NEW_EXTENSION(varstream, varstream.c, $ext_shared)  
fi  

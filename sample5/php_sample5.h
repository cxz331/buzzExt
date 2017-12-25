#ifndef PHP_SAMPLE5_H  
/* Prevent double inclusion */  
#define PHP_SAMPLE5_H  
  
/* Define Extension Properties */  
#define PHP_SAMPLE5_EXTNAME    "sample5"  
#define PHP_SAMPLE5_EXTVER    "1.0"  
  
/* Import configure options 
   when building outside of 
   the PHP source tree */  
#ifdef HAVE_CONFIG_H  
#include "config.h"  
#endif  
  
/* Include PHP Standard Header */  
#include "php.h"  
  
/* Define the entry point symbol 
 * Zend will use when loading this module 
 */  
extern zend_module_entry sample5_module_entry;  
#define phpext_sample5_ptr &sample5_module_entry  
  
#endif /* PHP_SAMPLE5_H */  

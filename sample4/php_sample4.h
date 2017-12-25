#ifndef PHP_SAMPLE4_H  
/* Prevent double inclusion */  
#define PHP_SAMPLE4_H  
  
/* Define Extension Properties */  
#define PHP_SAMPLE4_EXTNAME    "sample4"  
#define PHP_SAMPLE4_EXTVER    "1.0"  
  
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
extern zend_module_entry sample4_module_entry;  
#define phpext_sample4_ptr &sample4_module_entry  
  
#endif /* PHP_SAMPLE4_H */  

ZEND_BEGIN_MODULE_GLOBALS(sample4)  
    unsigned long counter;  
    const char *greeting;
    long mylong;  
	double mydouble;  
	zend_bool mybool;  
ZEND_END_MODULE_GLOBALS(sample4)

#ifdef ZTS  
#include "TSRM.h"  
#define SAMPLE4_G(v)    TSRMG(sample4_globals_id, zend_sample4_globals*, v)  
#else  
#define SAMPLE4_G(v)    (sample4_globals.v)  
#endif  
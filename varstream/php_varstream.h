#ifndef PHP_VARSTREAM_H
#define PHP_VARSTREAM_H

extern zend_module_entry varstream_module_entry;
#define phpext_varstream_ptr &varstream_module_entry

#ifdef PHP_WIN32
#   define PHP_VARSTREAM_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_VARSTREAM_API __attribute__ ((visibility("default")))
#else
#   define PHP_VARSTREAM_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(varstream);
PHP_MSHUTDOWN_FUNCTION(varstream);

#define PHP_VARSTREAM_WRAPPER       "var"
#define PHP_VARSTREAM_STREAMTYPE    "varstream"

/* 变量流的抽象数据结构 */
typedef struct _php_varstream_data {
    off_t   position;
    char    *varname;
    int     varname_len;
} php_varstream_data;

#ifdef ZTS
#define VARSTREAM_G(v) TSRMG(varstream_globals_id, zend_varstream_globals *, v)
#else
#define VARSTREAM_G(v) (varstream_globals.v)
#endif

#endif

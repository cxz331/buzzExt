#include "php_sample5.h"
#include "ext/standard/info.h"


PHP_FUNCTION(sample5_fopen)  
{  
    php_stream *stream;  
    char *path, *mode;  
    int path_len, mode_len;  
    int options = ENFORCE_SAFE_MODE | REPORT_ERRORS;  
  
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",  
            &path, &path_len, &mode, &mode_len) == FAILURE) {  
        return;  
    }  
    stream = php_stream_open_wrapper(path, mode, options, NULL);  
    if (!stream) {  
        RETURN_FALSE;  
    }  
    php_stream_to_zval(stream, return_value);  
} 

/* sockopen封装 */
PHP_FUNCTION(sample5_fsockopen)
{
    php_stream *stream;
    char *host, *transport, *errstr = NULL;
    int host_len, transport_len, implicit_tcp = 1, errcode = 0;
    long port = 0;
    int options = ENFORCE_SAFE_MODE;
    int flags = STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l",
                        &host, &host_len, &port) == FAILURE) {
        return;
    }
    if (port) {
        int implicit_tcp = 1;
        if (strstr(host, "://")) {
                /* A protocol was specified,
                 * no need to fall back on tcp:// */
            implicit_tcp = 0;
        }
        transport_len = spprintf(&transport, 0, "%s%s:%d",
                implicit_tcp ? "tcp://" : "", host, port);
    } else {
        /* When port isn't specified
         * we can safely assume that a protocol was
         * (e.g. unix:// or udg://) */
        transport = host;
        transport_len = host_len;
    }
    stream = php_stream_xport_create(transport, transport_len,
                              options, flags,
                              NULL, NULL, NULL, &errstr, &errcode);
    if (transport != host) {
        efree(transport);
    }
    if (errstr) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "[%d] %s",
                                errcode, errstr);
        efree(errstr);
    }
    if (!stream) {
        RETURN_FALSE;
    }
    php_stream_to_zval(stream, return_value);
}

/* 目录访问 */
PHP_FUNCTION(sample5_opendir)  
{  
    php_stream *stream;  
    char *path;  
    int path_len, options = ENFORCE_SAFE_MODE | REPORT_ERRORS;  
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",  
                                   &path, &path_len) == FAILURE) {  
        return;  
    }  
    stream = php_stream_opendir(path, options, NULL);  
    if (!stream) {  
        RETURN_FALSE;  
    }  
    php_stream_to_zval(stream, return_value);  
} 

static zend_function_entry php_sample5_functions[] = {
    PHP_FE(sample5_fopen, NULL)
    PHP_FE(sample5_fsockopen, NULL)
    PHP_FE(sample5_opendir, NULL)
    { NULL, NULL, NULL }
};

PHP_MINIT_FUNCTION(sample5)
{
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(sample5)
{
    return SUCCESS;
}

PHP_RINIT_FUNCTION(sample5)
{
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(sample5)
{
    return SUCCESS;
}
PHP_MINFO_FUNCTION(sample5)
{
}

zend_module_entry sample5_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_SAMPLE5_EXTNAME,
    php_sample5_functions,
    PHP_MINIT(sample5),
    PHP_MSHUTDOWN(sample5),
    PHP_RINIT(sample5),
    PHP_RSHUTDOWN(sample5),
    PHP_MINFO(sample5),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_SAMPLE5_EXTVER,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SAMPLE5
ZEND_GET_MODULE(sample5)
#endif


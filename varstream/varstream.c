#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/url.h"
#include "php_varstream.h"

static size_t php_varstream_write(php_stream *stream,
                const char *buf, size_t count TSRMLS_DC)
{
    php_varstream_data *data = stream->abstract;
    zval **var;
    size_t newlen;

    /* 查找变量 */
    if (zend_hash_find(&EG(symbol_table), data->varname,
            data->varname_len + 1,(void**)&var) == FAILURE) {
        /* 变量不存在, 直接创建一个字符串类型的变量, 并保存新传递进来的内容 */
       zval *newval;
       MAKE_STD_ZVAL(newval);
       ZVAL_STRINGL(newval, buf, count, 1); 
       /* 将新的zval *放到变量中 */
       zend_hash_add(&EG(symbol_table), data->varname,
           data->varname_len + 1, (void*)&newval,
           sizeof(zval*), NULL);
       return count;
    }   
    /* 如果需要, 让变量可写. 这里实际上处理的是写时复制 */
    SEPARATE_ZVAL_IF_NOT_REF(var);
    /* 转换为字符串类型 */
    convert_to_string_ex(var);
    /* 重置偏移量(译注: 相比于正常的文件系统, 这里的处理实际上不支持文件末尾的空洞创建, 读者如果熟悉*nix文件系统, 应该了解译者所说, 否则请略过) */
    if (data->position > Z_STRLEN_PP(var)) {
        data->position = Z_STRLEN_PP(var);
    }   
    /* 计算新的字符串长度 */
    newlen = data->position + count;
    if (newlen < Z_STRLEN_PP(var)) {
        /* 总长度不变 */
        newlen = Z_STRLEN_PP(var);
    } else if (newlen > Z_STRLEN_PP(var)) {
        /* 重新调整缓冲区大小以保存新内容 */
        Z_STRVAL_PP(var) =erealloc(Z_STRVAL_PP(var),newlen+1);
        /* 更新字符串长度 */
        Z_STRLEN_PP(var) = newlen;
        /* 确保字符串NULL终止 */
        Z_STRVAL_PP(var)[newlen] = 0;
    }   
    /* 将数据写入到变量中 */
    memcpy(Z_STRVAL_PP(var) + data->position, buf, count);
    data->position += count;

    return count;
}

static size_t php_varstream_read(php_stream *stream,
                char *buf, size_t count TSRMLS_DC)
{
    php_varstream_data *data = stream->abstract;
    zval **var, copyval;
    int got_copied = 0;
    size_t toread = count;

    if (zend_hash_find(&EG(symbol_table), data->varname,
        data->varname_len + 1, (void**)&var) == FAILURE) {
        /* 变量不存在, 读不到数据, 返回0字节长度 */
        return 0;
    }   
    copyval = **var;
    if (Z_TYPE(copyval) != IS_STRING) {
        /* 对于非字符串类型变量, 创建一个副本进行读, 这样对于只读的变量, 就不会改变其原始类型 */
        zval_copy_ctor(&copyval);
        INIT_PZVAL(&copyval);
        got_copied = 1;
    }   
    if (data->position > Z_STRLEN(copyval)) {
        data->position = Z_STRLEN(copyval);
    }   
    if ((Z_STRLEN(copyval) - data->position) < toread) {
        /* 防止读取到变量可用缓冲区外的内容 */
        toread = Z_STRLEN(copyval) - data->position;
    }   
    /* 设置缓冲区 */
    memcpy(buf, Z_STRVAL(copyval) + data->position, toread);
    data->position += toread;

    /* 如果创建了副本, 则释放副本 */
    if (got_copied) {
        zval_dtor(&copyval);
    }   

    /* 返回设置到缓冲区的字节数 */
    return toread;
}

static int php_varstream_closer(php_stream *stream,
                            int close_handle TSRMLS_DC)
{
    php_varstream_data *data = stream->abstract;

    /* 释放内部结构避免泄露 */
    efree(data->varname);
    efree(data);

    return 0;
}

static int php_varstream_flush(php_stream *stream TSRMLS_DC)
{
    php_varstream_data *data = stream->abstract;
    zval **var;

    /* 根据不同情况, 重置偏移量 */
    if (zend_hash_find(&EG(symbol_table), data->varname,
                    data->varname_len + 1, (void**)&var)
                    == SUCCESS) {
        if (Z_TYPE_PP(var) == IS_STRING) {
            data->position = Z_STRLEN_PP(var);
        } else {
            zval copyval = **var;
            zval_copy_ctor(&copyval);
            convert_to_string(&copyval);
            data->position = Z_STRLEN(copyval);
            zval_dtor(&copyval);
        }
    } else {
        data->position = 0;
    }

    return 0;
}

static int php_varstream_seek(php_stream *stream, off_t offset,
                    int whence, off_t *newoffset TSRMLS_DC)
{
    php_varstream_data *data = stream->abstract;

    switch (whence) {
        case SEEK_SET:
            data->position = offset;
            break;
        case SEEK_CUR:
            data->position += offset;
            break;
        case SEEK_END:
        {
            zval **var;
           size_t curlen = 0;

           if (zend_hash_find(&EG(symbol_table),
                   data->varname,    data->varname_len + 1,
                   (void**)&var) == SUCCESS) {
              if (Z_TYPE_PP(var) == IS_STRING) {
                  curlen = Z_STRLEN_PP(var);
              } else {
                  zval copyval = **var;
                  zval_copy_ctor(&copyval);
                  convert_to_string(&copyval);
                  curlen = Z_STRLEN(copyval);
                  zval_dtor(&copyval);
              }
           }

           data->position = curlen + offset;
           break;
       }
    }

    /* 防止随机访问指针移动到缓冲区开始位置之前 */
    if (data->position < 0) {
        data->position = 0;
    }

    if (newoffset) {
        *newoffset = data->position;
    }

    return 0;
}

static php_stream_ops php_varstream_ops = {
    php_varstream_write,
    php_varstream_read,
    php_varstream_closer,
    php_varstream_flush,
    PHP_VARSTREAM_STREAMTYPE,
    php_varstream_seek,
    NULL, /* cast */
    NULL, /* stat */
    NULL, /* set_option */
};

/* Define the wrapper operations */
static php_stream *php_varstream_opener(
            php_stream_wrapper *wrapper,
            char *filename, char *mode, int options,
            char **opened_path, php_stream_context *context
            STREAMS_DC TSRMLS_DC)
{
    php_varstream_data *data;
    php_url *url;
    /* 判断是否为持久化资源，如果是则报错 */
    if (options & STREAM_OPEN_PERSISTENT) {
        /* 按照变量流的定义, 是不能持久化的
         * 因为变量在请求结束后将被释放
         */
        php_stream_wrapper_log_error(wrapper, options
            TSRMLS_CC, "Unable to open %s persistently",
                                        filename);
        return NULL;
    }

    /* 标准URL解析: scheme://user:pass@host:port/path?query#fragment */
    url = php_url_parse(filename);
    if (!url) {
        php_stream_wrapper_log_error(wrapper, options
            TSRMLS_CC, "Unexpected error parsing URL");
        return NULL;
    }
    
    /* 检查是否有变量流URL必须的元素host, 以及scheme是否是var */
    if (!url->host || (url->host[0] == 0) ||
        strcasecmp("var", url->scheme) != 0) {
        /* Bad URL or wrong wrapper */
        php_stream_wrapper_log_error(wrapper, options
            TSRMLS_CC, "Invalid URL, must be in the form: "
                     "var://variablename");
        php_url_free(url);
        return NULL;
    }

    /* 创建一个数据结构保存协议信息(变量流协议重要是变量名, 变量名长度, 当前偏移量) */
    data = emalloc(sizeof(php_varstream_data));
    data->position = 0;
    data->varname_len = strlen(url->host);
    data->varname = estrndup(url->host, data->varname_len + 1);
    /* 释放前面解析出来的url占用的内存 */
    php_url_free(url);

    /* 实例化一个流, 为其赋予恰当的流ops, 绑定抽象数据 */
    return php_stream_alloc(&php_varstream_ops, data, 0, mode);
}

/* 这个只是下面结构体中的ops的结构体定义 */
static php_stream_wrapper_ops php_varstream_wrapper_ops = {
    php_varstream_opener, /* 调用php_stream_open_wrapper(sprintf("%s://xxx", PHP_VARSTREAM_WRAPPER))时执行 */
    NULL, /* stream_close */
    NULL, /* stream_stat */
    NULL, /* url_stat */
    NULL, /* dir_opener */
    PHP_VARSTREAM_WRAPPER,
    NULL, /* unlink */
#if PHP_MAJOR_VERSION >= 5
    /* PHP >= 5.0 only */
    NULL, /* rename */
    NULL, /* mkdir */
    NULL, /* rmdir */
#endif
};

/* 第二步， 
1.将var协议和我们定义的包装器关联起来
2.调用用户空间函数比如fopen将通过这个包装器的ops元素得到php_varstream_wrapper_ops, 这样去调用流的打开函数php_varstream_opener 
3.php_varstream_opener函数在上面
*/
static php_stream_wrapper php_varstream_wrapper = {
    &php_varstream_wrapper_ops, /* 自己定义的包装器 */
    NULL, /* abstract */
    0, /* is_url */
};

/* 第一步，注册包装器到php内核的钩子函数 */
PHP_MINIT_FUNCTION(varstream)
{
    /* 注册流包装器:
     * 1. 检查流包装器名字是否正确(符合这个正则: /^[a-zA-Z0-9+.-]+$/)
     * 2. 将传入的php_varstream_wrapper增加到url_stream_wrappers_hash这个HashTable中, key为PHP_VARSTREAM_WRAPPER
     */
    if (php_register_url_stream_wrapper(PHP_VARSTREAM_WRAPPER,
            &php_varstream_wrapper TSRMLS_CC)==FAILURE) {
        return FAILURE;
    }
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(varstream)
{
    /* 卸载流包装器: 从url_stream_wrappers_hash中删除 */
    if (php_unregister_url_stream_wrapper(PHP_VARSTREAM_WRAPPER
                                TSRMLS_CC) == FAILURE) {
        return FAILURE;
    }
    return SUCCESS;
}

zend_module_entry varstream_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "varstream",
    NULL,
    PHP_MINIT(varstream),
    PHP_MSHUTDOWN(varstream),
    NULL,
    NULL,
    NULL,
#if ZEND_MODULE_API_NO >= 20010901
    "0.1",
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_VARSTREAM
ZEND_GET_MODULE(varstream)
#endif

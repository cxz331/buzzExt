#include "php_sample4.h"
#include "ext/standard/info.h"
#include "php_ini.h"
#include "SAPI.h" /* needed for sapi_module */ 

ZEND_DECLARE_MODULE_GLOBALS(sample4); 

/* 展示字符串值的处理器 */
PHP_INI_DISP(php_sample4_display_greeting)  
{  
    const char *value = ini_entry->value;  
  
    /* 选择合适的当前值或原始值 */  
    if (type == ZEND_INI_DISPLAY_ORIG &&  
        ini_entry->modified) {  
        value = ini_entry->orig_value;  
    }

    /* 使得打招呼的字符串粗体显示(当以HTML方式输出时) */  
    if (sapi_module.phpinfo_as_text) {  
        php_printf("%s", value);  
    } else {  
        php_printf("<b>%s</b>", value);  
    }  
} 

/* 这是底下配置项的一个回调函数，如果新设置的值为0，则返回失败 */
 

PHP_INI_BEGIN()  

    STD_PHP_INI_ENTRY("default_socket_timeout", "60", PHP_INI_ALL, OnUpdateLong, mylong, zend_sample4_globals, sample4_globals)

PHP_INI_END()

static void php_sample4_globals_ctor(zend_sample4_globals *sample4_globals TSRMLS_DC)  
{  
    /* 在线程产生时初始化一个新的zend_sample4_globals结构体 */  
    sample4_globals->counter = 0;  
}  
static void php_sample4_globals_dtor(zend_sample4_globals *sample4_globals TSRMLS_DC)  
{  
    /* 在初始化阶段分配的各种资源, 都在这里释放 */ 

}

zend_bool php_sample4_autoglobal_callback(const char *name,  
                            uint name_len TSRMLS_DC)  
{  
    zval *sample4_val;  
    int i;  
  
    MAKE_STD_ZVAL(sample4_val);  
    array_init(sample4_val);  
    for(i = 0; i < 10000; i++) {  
        add_next_index_long(sample4_val, i);  
    }  
    ZEND_SET_SYMBOL(&EG(symbol_table), "_SAMPLE4",  
                                    sample4_val);  
    return 0;  
}  

PHP_MINIT_FUNCTION(sample4)
{
    /* ini的配置 */
    REGISTER_INI_ENTRIES();
    /* 线程环境下 - 引擎将在每个新的线程产生时分配一块内存 专门用于线程服务请求时的似有存储空间 */
    #ifdef ZTS  
        ts_allocate_id(&sample4_globals_id,  
                    sizeof(zend_sample4_globals),  
                    (ts_allocate_ctor)php_sample4_globals_ctor,  
                    NULL);  
    #else  
        php_sample4_globals_ctor(&sample4_globals TSRMLS_CC);  
    #endif 

    /* 只是声明了一个常量 */
    REGISTER_STRING_CONSTANT("SAMPLE4_VERSION",  
            PHP_SAMPLE4_EXTVER, CONST_CS | CONST_PERSISTENT);  

    /* 声明超全局变量 */
    zend_register_auto_global("_SAMPLE4", sizeof("_SAMPLE4") - 1, false, php_sample4_autoglobal_callback TSRMLS_CC);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(sample4)
{
    /* ini配置关闭 */
    UNREGISTER_INI_ENTRIES();
    #ifndef ZTS  
        php_sample4_globals_dtor(&sample4_globals TSRMLS_CC);  
    #endif  
    return SUCCESS; 
}

PHP_RINIT_FUNCTION(sample4)
{
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(sample4)
{
    return SUCCESS;
}

PHP_MINFO_FUNCTION(sample4)  
{  
    DISPLAY_INI_ENTRIES();
} 

PHP_FUNCTION(sample4_hello_world)  
{  
    //const char *greeting = INI_STR("sample4.longval");  
    //php_printf("%s\n", greeting);  
} 

PHP_FUNCTION(sample4_counter)  
{  
    RETURN_LONG(++SAMPLE4_G(counter));  
} 

static zend_function_entry php_sample4_functions[] = {
    PHP_FE(sample4_hello_world, NULL)
    PHP_FE(sample4_counter, NULL)
    { NULL, NULL, NULL }
};

zend_module_entry sample4_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_SAMPLE4_EXTNAME,
    php_sample4_functions,
    PHP_MINIT(sample4),
    PHP_MSHUTDOWN(sample4),
    PHP_RINIT(sample4),
    PHP_RSHUTDOWN(sample4),
    PHP_MINFO(sample4),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_SAMPLE4_EXTVER,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SAMPLE4
ZEND_GET_MODULE(sample4)
#endif


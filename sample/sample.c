#include "php_sample.h"  

/* 实例3 */
#if (PHP_MAJOR_VERSION > 5) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 0)  
PHP_FUNCTION(sample_reference_a)  
{  
    zval **a_ptr, *a, *b;   
    int a_pp;
    /* 从全局符号表查找变量$a */  
    if (zend_hash_find(&EG(symbol_table), "a", sizeof("a"), (void**)&a_ptr) == SUCCESS) {  
        a = *a_ptr;  
    } else {  
        /* 如果不存在$GLOBALS['a']则创建它 */  
        ALLOC_INIT_ZVAL(a);
        zend_hash_add(&EG(symbol_table), "a", sizeof("a"), &a, sizeof(zval*), NULL); 
    }  
    /* 废弃旧的返回值 */  
    zval_ptr_dtor(return_value_ptr);  
    if (!(Z_ISREF_P(a)) && Z_REFCOUNT_P(a) > 1) {  
        /* $a需要写时复制, 在使用之前, 必选先隔离 */  
        zval *newa;  
        MAKE_STD_ZVAL(newa);  
        *newa = *a;  
        zval_copy_ctor(newa);   
        Z_SET_ISREF_P(newa);
        Z_SET_REFCOUNT_P(newa, 1); 
        zend_hash_update(&EG(symbol_table), "a", sizeof("a"), &newa,  
                                                 sizeof(zval*), NULL);  
        a = newa;  
    }  
    /* 将新的返回值设置为引用方式并增加refcount */  
    Z_SET_ISREF_P(a);
    Z_ADDREF_P(a);
    a_pp = Z_REFCOUNT_P(a);
    Z_SET_REFCOUNT_P(a, a_pp + 1);
    *return_value_ptr = a;  
}  
#endif /* PHP >= 5.1.0 */  

PHP_FUNCTION(sample_hello_world)  
{  
    php_printf("Hello World!\n");  
}

PHP_FUNCTION(sample_long)
{
    ZVAL_LONG(return_value, 998);
    return;   
}

/*实例4*/
PHP_FUNCTION(sample_byref_calltime)  
{  
    zval *a;  
    int addtl_len = sizeof(" (modified by ref!)") - 1;  
  
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &a) == FAILURE) {  
        RETURN_NULL();  
    }
    /* 确保变量是字符串 */  
    convert_to_string(a);  
    /* 扩大a的缓冲区以使可以保存要追加的数据 */  
    Z_STRVAL_P(a) = erealloc(Z_STRVAL_P(a), Z_STRLEN_P(a) + addtl_len + 1);
    memcpy(Z_STRVAL_P(a) + Z_STRLEN_P(a), " (modified by ref!)", addtl_len + 1);  
    Z_STRLEN_P(a) += addtl_len;  
}  


PHP_FUNCTION(sample_getlong)  
{  
    long foo;  
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,  
                         "l", &foo) == FAILURE) {  
        RETURN_NULL();  
    }  
    php_printf("你丫输入的参数是这个: %ld\n", foo);  
    RETURN_TRUE;  
}  


PHP_FUNCTION(sample_hello_world_ext)  
{  
    char *name;  
    int name_len;  
    char *greeting = "Mr./Mrs.";  
    int greeting_len = sizeof("Mr./Mrs.") - 1;  
  
  
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s",  
      &name, &name_len, &greeting, &greeting_len) == FAILURE) {  
        RETURN_NULL();  
    }  
    php_printf("Hello ");  
    PHPWRITE(greeting, greeting_len);  
    php_printf(" ");  
    PHPWRITE(name, name_len);  
    php_printf("!\n");  
}  

/* 实现count() */
PHP_FUNCTION(sample_count_array)  
{  
    zval *arr;  
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a",  
                                    &arr) == FAILURE) {  
        RETURN_NULL();  
    }  
    RETURN_LONG(zend_hash_num_elements(Z_ARRVAL_P(arr)));  
}  

/* hashtable */
void zend_hash_display_string(const HashTable *ht)
{
    Bucket *p; 
    uint i;

    if (UNEXPECTED(ht->nNumOfElements == 0)) {
        zend_output_debug_string(0, "The hash is empty");
        return;
    }   
    for (i = 0; i < ht->nTableSize; i++) {
        p = ht->arBuckets[i];
        while (p != NULL) {
            zend_output_debug_string(0, "%s[0x%lX] <==> %s", p->arKey, p->h, (char *)p->pData);
            p = p->pNext;
        }   
    }   

    p = ht->pListTail;
    while (p != NULL) {
        zend_output_debug_string(0, "%s[hash = 0x%lX, pointer = %p] <==> %s[pointer = %p]", p->arKey, p->h, p->arKey, (char *)p->pData, p->pData);
        p = p->pListLast;
    }   
}
PHP_FUNCTION(sample_ht)
{
    HashTable   *ht0;
    char        *key;
    char        *value;
    void        *pDest; /* pDest 返回的是插入 */

    key     = emalloc(16);
    value   = emalloc(32);

    ALLOC_HASHTABLE(ht0);
    zend_hash_init(ht0, 50, NULL, NULL, 0); 

    strcpy(key, "ABCDEFG");
    strcpy(value, "0123456789");

    printf("key: %p %s\n", key, key);
    printf("value: %p %s\n", value, value);

    zend_hash_add(ht0, key, 8, value, 11, &pDest);

    printf("pDest: %p\n", pDest);

    zend_hash_display_string(ht0);

    zend_hash_destroy(ht0);
    FREE_HASHTABLE(ht0);

    efree(value);
    efree(key);

    RETURN_NULL();
}

/* 函数逻辑实现 */  
PHP_FUNCTION(sample_array_copy)  
{  
    zval    *a1, *a2, **z;  
    char    *key;  
    int     key_len;  
    ulong   h;    
  
    if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "aas", &a1, &a2, &key, &key_len) == FAILURE ) {
        RETURN_FALSE;  
    }     
  
    h   = zend_get_hash_value(key, key_len + 1);
  
    if ( zend_hash_quick_find(Z_ARRVAL_P(a1), key, key_len + 1, h, (void **)&z) == FAILURE ) {  
        RETURN_FALSE;  
    }     
  
    Z_SET_REFCOUNT_PP(z, Z_REFCOUNT_PP(z) + 1);   
    Z_SET_ISREF_PP(z); /* 这里设置为引用类型, 读者可以注释这一行比较结果, 增强对变量引用的理解. */  
    zend_hash_quick_update(Z_ARRVAL_P(a2), key, key_len + 1, h, z, sizeof(zval *), NULL);  
  
    RETURN_TRUE;  
}  

/* 函数功能：返回个数组，值为传入的参数 */
PHP_FUNCTION(sample_return_arr){
    char     *key1, *key2;
    int      key1_len, key2_len;
    if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &key1, &key1_len, &key2, &key2_len) == FAILURE ) {
        RETURN_FALSE;  
    }
    array_init(return_value);
    zval* arr;
    ALLOC_INIT_ZVAL(arr);
    array_init(arr);
    add_assoc_long(arr,key1,121);
    add_assoc_string(arr,"string","hello world!",1);
    add_assoc_zval(return_value,"array",arr);
}

/* 函数功能：添加一些数据到数组中 */
PHP_FUNCTION(sample_array)  
{  
    zval *subarray;  
  
    array_init(return_value);  
    /* 增加一些标量值 */  
    add_assoc_long(return_value, "life", 42);  
    add_index_bool(return_value, 123, 1);  
    add_next_index_double(return_value, 3.1415926535);  
    /* 增加一个静态字符串, 由php去复制 */  
    add_next_index_string(return_value, "Foo", 1);  
    /* 手动复制的字符串 */  
    add_next_index_string(return_value, estrdup("Bar"), 0);  
  
    /* 创建一个子数组 */  
    MAKE_STD_ZVAL(subarray);  
    array_init(subarray);  
    /* 增加一些数值 */  
    add_next_index_long(subarray, 1);  
    add_next_index_long(subarray, 20);  
    add_next_index_long(subarray, 300);  
    /* 将子数组放入到父数组中 */  
    add_index_zval(return_value, 444, subarray);  
}  

/* ---- 资源类型 ---- */

/* 析构函数 */
static void php_sample_descriptor_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)  
{  
    FILE *fp = (FILE*)rsrc->ptr;  
    fclose(fp);  
}  

/* 持久化资源 */
static void php_sample_descriptor_dtor_persistent(zend_rsrc_list_entry *rsrc TSRMLS_DC)  
{  
    php_sample_descriptor_data *fdata =  
                (php_sample_descriptor_data*)rsrc->ptr;  
    fclose(fdata->fp);  
    pefree(fdata->filename, 1);  
    pefree(fdata, 1);  
}  

static int le_sample_descriptor;  
PHP_MINIT_FUNCTION(sample)  
{  
    /* 该函数返回的是定义好的资源在hashTable中的index - 全局资源表 */
    /* 
     *  1.第一个指针指向的函数在一个请求生命周期内注册资源的最后一个引用被破坏时调用. 实际上就是我们所说的在已存储的资源变量上调用unset(). 
     *  2.第二个指针指向另外一个回调函数, 它用于持久化资源, 当一个进程或线程终止时被调用. 本章后面将会介绍持久化资源.
     *  3.var_dump()时返回的名称
     *  4.模块号，在启动函数中可以获取该值
     *  */
    le_sample_descriptor = zend_register_list_destructors_ex(  
                php_sample_descriptor_dtor, NULL, PHP_SAMPLE_DESCRIPTOR_RES_NAME,  
                module_number); 

    le_sample_descriptor_persist =  zend_register_list_destructors_ex( NULL, php_sample_descriptor_dtor_persistent, PHP_SAMPLE_DESCRIPTOR_RES_NAME, module_number);
    return SUCCESS;  
}  

/* fopen的重新实现 */

PHP_FUNCTION(sample_fopen_oo)  
{  
    FILE *fp;  
    char *filename, *mode;  
    int filename_len, mode_len;  
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",  
                        &filename, &filename_len,  
                        &mode, &mode_len) == FAILURE) {  
        RETURN_NULL();  
    }  
    /* filename, mode 2个参数不能为空 */
    if (!filename_len || !mode_len) {  
        php_error_docref(NULL TSRMLS_CC, E_WARNING,  
                "Invalid filename or mode length");  
        RETURN_FALSE;  
    }  
    /* 调用fopen */
    fp = fopen(filename, mode);  
    if (!fp) {  
        php_error_docref(NULL TSRMLS_CC, E_WARNING,  
                "Unable to open %s using mode %s",  
                filename, mode);  
        RETURN_FALSE;  
    }  
    /* 第一个参数：zval的数据，第二个参数：一个指针或者index，第三个参数：注册的全局资源的指针 */
    /* 简单的说就是:把一个指针(得是资源吧)放入注册的资源类型的hashTable中，返回的指针赋值给return_value并将
     * return_value的类型设置为is_resource
     */
    ZEND_REGISTER_RESOURCE(return_value, fp,  
                                le_sample_descriptor);  
}  

PHP_FUNCTION(sample_fopen)
{
    php_sample_descriptor_data *fdata;
    FILE *fp;
    char *filename, *mode;
    int filename_len, mode_len;
    zend_bool persist = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss|b",
                &filename, &filename_len, &mode, &mode_len,
                &persist) == FAILURE) {
        RETURN_NULL();
    }
    if (!filename_len || !mode_len) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                "Invalid filename or mode length");
        RETURN_FALSE;
    }
    fp = fopen(filename, mode);
    if (!fp) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                "Unable to open %s using mode %s",
                filename, mode);
        RETURN_FALSE;
    }
    if (!persist) {
        fdata = emalloc(sizeof(php_sample_descriptor_data));
        fdata->filename = estrndup(filename, filename_len);
        fdata->fp = fp;
        ZEND_REGISTER_RESOURCE(return_value, fdata,
                                le_sample_descriptor);
    } else {
        zend_rsrc_list_entry le;
        char *hash_key;
        int hash_key_len;

        fdata =pemalloc(sizeof(php_sample_descriptor_data),1);
        fdata->filename = pemalloc(filename_len + 1, 1);
        memcpy(fdata->filename, filename, filename_len + 1);
        fdata->fp = fp;
        ZEND_REGISTER_RESOURCE(return_value, fdata,
                        le_sample_descriptor_persist);

        /* 在persistent_list中保存一份拷贝 */
        le.type = le_sample_descriptor_persist;
        le.ptr = fdata;
        hash_key_len = spprintf(&hash_key, 0,
                "sample_descriptor:%s:%s", filename, mode);
        zend_hash_update(&EG(persistent_list),
            hash_key, hash_key_len + 1,
            (void*)&le, sizeof(zend_rsrc_list_entry), NULL);
        efree(hash_key);
    }
}

/* fwrite的实现 */
PHP_FUNCTION(sample_fwrite) 
{  
    FILE *fp;  
    zval *file_resource;  
    char *data;  
    int data_len;  
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs",  
            &file_resource, &data, &data_len) == FAILURE ) {  
        RETURN_NULL();  
    }  
    /* 使用zval *验证资源类型, 并从注册资源表中取回它的指针 */  
    /* 1.首个参数用于接收handle值，
       2.第二个参数是handle值的类型，这个函数会扩展成“rsrc = (rsrc_type) zend_fetch_resource(…)”，因此应该保证rsrc是rsrc_type类型的；
       3.file_resource是用户传入的zval，这里使用zval**类型，函数从中取得资源id；
       4.default_id用来直接指定资源id，如果该值不是-1，则使用它，并且忽略passed_id，所以通常应该使用-1；
       5.resource_type_name是资源名称，当获取资源失败时，函数使用它来输出错误信息；
       6.resource_type是资源类型，如果取得的资源不是该类型的，则函数返回NULL，这用于防止用户传入一个其他类型资源的zval */
    ZEND_FETCH_RESOURCE(fp, FILE*, &file_resource, -1,  
        PHP_SAMPLE_DESCRIPTOR_RES_NAME, le_sample_descriptor);  
    /* 写数据并返回实际写入到文件的字节数 */  
    RETURN_LONG(fwrite(data, 1, data_len, fp));  
}

/* 析构函数 - 删除资源 */
PHP_FUNCTION(sample_fclose)  
{  
    FILE *fp;  
    zval *file_resource;  
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",  
                        &file_resource) == FAILURE ) {  
        RETURN_NULL();  
    }  
    /* 虽然并不需要真的取回FILE *资源, 但执行这个宏可以去检查我们关闭资源类型是否正确 */  
    ZEND_FETCH_RESOURCE(fp, FILE*, &file_resource, -1,  
        PHP_SAMPLE_DESCRIPTOR_RES_NAME, le_sample_descriptor);  
    /* 强制资源进入自解模式 */  
    zend_hash_index_del(&EG(regular_list),  
                    Z_RESVAL_P(file_resource));  
    RETURN_TRUE;  
} 

/* 以上均为实例部分 */
 
static zend_function_entry php_sample_functions[] = {  
    PHP_FE(sample_hello_world, NULL)
    PHP_FE(sample_long, NULL) 
    PHP_FE(sample_byref_calltime, NULL)
    #if (PHP_MAJOR_VERSION > 5) || (PHP_MAJOR_VERSION == 5 && \  
                          PHP_MINOR_VERSION > 0)  
    PHP_FE(sample_reference_a, php_sample_retref_arginfo)  
    #endif /* PHP >= 5.1.0 */  
    /*参数设置引用传值版本*/
    PHP_FALIAS(sample_byref_compiletime, sample_byref_calltime, php_sample_byref_arginfo) 
    PHP_FE(sample_getlong, NULL)
    PHP_FE(sample_hello_world_ext, NULL)
    PHP_FE(sample_count_array, NULL)
    PHP_FE(sample_ht, NULL)
    PHP_FE(sample_array_copy, sample_array_copy_arginfo)
    PHP_FE(sample_return_arr, NULL)
    PHP_FE(sample_fopen, NULL)
    PHP_FE(sample_fclose, NULL)
    PHP_FE(sample_fopen_oo, NULL)

    { NULL, NULL, NULL }  
};  
 
zend_module_entry sample_module_entry = {  
#if ZEND_MODULE_API_NO >= 20010901  
     STANDARD_MODULE_HEADER,  
#endif  
    PHP_SAMPLE_EXTNAME,  
    php_sample_functions, /* Functions */  
    PHP_MINIT(sample), /* MINIT */  
    NULL, /* MSHUTDOWN */  
    NULL, /* RINIT */  
    NULL, /* RSHUTDOWN */  
    NULL, /* MINFO */  
#if ZEND_MODULE_API_NO >= 20010901  
    PHP_SAMPLE_EXTVER,  
#endif  
    STANDARD_MODULE_PROPERTIES  
};  
  
#ifdef COMPILE_DL_SAMPLE  
ZEND_GET_MODULE(sample)  
#endif  

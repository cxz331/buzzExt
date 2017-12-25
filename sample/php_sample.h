#ifndef PHP_SAMPLE_H  
/* 防止重复包含 */  
#define PHP_SAMPLE_H  
  
/* 定义扩展的属性 */  
#define PHP_SAMPLE_EXTNAME    "sample"  
#define PHP_SAMPLE_EXTVER    "1.0"  
#define PHP_SAMPLE_DESCRIPTOR_RES_NAME "File Descriptor"  
  
/* 在php源码树外面构建时引入配置选项 */  
#ifdef HAVE_CONFIG_H  
#include "config.h"  
#endif  
  
/* 包含php的标准头文件 */  
#include "php.h"  
  
/* 定义入口点符号, Zend在加载这个模块的时候使用 */  
extern zend_module_entry sample_module_entry;  
#define phpext_sample_ptr &sample_module_entry  
  
#endif /* PHP_SAMPLE_H */  

#if (PHP_MAJOR_VERSION > 5) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 0)  
    ZEND_BEGIN_ARG_INFO_EX(php_sample_retref_arginfo, 0, 1, 0)  
    ZEND_END_ARG_INFO ()  
#endif /* PHP >= 5.1.0 */  

/* 定义常量结构，php_sample_byref_arginfo为函数接受参数时的处理(是否引用传值，参数类型等) */
#ifdef ZEND_ENGINE_2  
static  
    ZEND_BEGIN_ARG_INFO(php_sample_byref_arginfo, 0)  
        ZEND_ARG_PASS_INFO(1)  
    ZEND_END_ARG_INFO()  
#else /* ZE 1 */  
static unsigned char php_sample_byref_arginfo[] = { 1, BYREF_FORCE };  
#endif  

/* php_sample.h中定义的arg info */  
#ifdef ZEND_ENGINE_2  
	ZEND_BEGIN_ARG_INFO(sample_array_copy_arginfo, 0)  
	    ZEND_ARG_ARRAY_INFO(1, "a", 0)  
	    ZEND_ARG_ARRAY_INFO(1, "b", 0)  
	    ZEND_ARG_PASS_INFO(0)  
	ZEND_END_ARG_INFO()  
#else  
static unsigned char sample_array_copy_arginfo[] = {3, BYREF_FORCE, BYREF_FORCE, 0};  
#endif  

/* 想象一下, 现在还需要和FILE *指针一起保存打开文件的文件名. 现在, 你就需要在php_sample.h中创建一个自定义结构体来保存这个联合信息 */
typedef struct _php_sample_descriptor_data {  
    char *filename;  
    FILE *fp;  
} php_sample_descriptor_data; 

static int le_sample_descriptor_persist;  













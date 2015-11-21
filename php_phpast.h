
#ifndef PHP_PHPAST_H
#define PHP_PHPAST_H

extern zend_module_entry phpast_module_entry;
#define phpext_phpast_ptr &phpast_module_entry

#define PHP_PHPAST_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_PHPAST_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_PHPAST_API __attribute__ ((visibility("default")))
#else
#	define PHP_PHPAST_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(phpast) /* {{{ */
    zval hook_callable;
    zval compiled_ast;
ZEND_END_MODULE_GLOBALS(phpast)
/* }}} */

/* Always refer to the globals in your function as PHPAST_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
#define PHPAST_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(phpast, v)

#if defined(ZTS) && defined(COMPILE_DL_PHPAST)
ZEND_TSRMLS_CACHE_EXTERN();
#endif

static void register_classes();
static void create_phpast_from_zend_ast(zval *obj, zend_ast *ast);

void hook_ast_process();
void unhook_ast_process();
void php_ast_process(zend_ast *ast);

PHP_METHOD(PHPAst, __construct);
PHP_METHOD(PHPAst, __destruct);
PHP_METHOD(PHPAst, eachChild);
PHP_METHOD(PHPAst, getChildCount);
PHP_METHOD(PHPAst, getKind);
PHP_METHOD(PHPAst, getKindName);
PHP_METHOD(PHPAst, isZval);
PHP_METHOD(PHPAst, getZval);
PHP_METHOD(PHPAst, export);
PHP_METHOD(PHPAst, eval);
PHP_METHOD(PHPAst, __toString);
PHP_METHOD(PHPAst, enableAstHook);
PHP_METHOD(PHPAst, disableAstHook);
PHP_METHOD(PHPAst, compileFile);
PHP_METHOD(PHPAst, compileString);


typedef struct _phpast_obj {
    zend_ast    *ast;
    char         is_owner;
    zend_array   children;
    zend_object  std;
} phpast_obj;

static inline phpast_obj *phpast_obj_from_obj(zend_object *obj) {
    return (phpast_obj*) ((char*)(obj) - XtOffsetOf(phpast_obj, std));
}

#define Z_PHPAST_P(zv) phpast_obj_from_obj(Z_OBJ_P((zv)))

/* phpast_kind.c */
void phpast_declare_kind_constant(zend_class_entry *ce);
const char *phpast_get_kind_name(zend_ast_kind kind);

#endif	/* PHP_PHPAST_H */


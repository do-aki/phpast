
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/spl/spl_exceptions.h"
#include "php_phpast.h"

#include <zend_language_parser.h>

ZEND_DECLARE_MODULE_GLOBALS(phpast)

/* True global resources - no need for thread safety here */
static int le_phpast;
static zend_class_entry *ce_phpast;
static zend_object_handlers phpast_object_handlers;
zend_ast_process_t original_ast_process = NULL;

static zend_bool ast_kind_is_decl(zend_ast_kind kind);
static zend_bool ast_kind_is_list(zend_ast_kind kind);
static uint32_t ast_kind_get_num_children(zend_ast_kind kind);
static void ast_destroy(zend_ast *ast);
static zend_object *phpast_new(zend_class_entry *class_type);
static void phpast_free(zend_object *object);
static char *phpast_zval_to_string(zval *zv);
static char *phpast_to_string(phpast_obj *self);

static zend_ast *create_zend_ast_from_phpast(phpast_obj *obj);


/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("phpast.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_phpast_globals, phpast_globals)
    STD_PHP_INI_ENTRY("phpast.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_phpast_globals, phpast_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_phpast_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_phpast_compiled)
{
	char *arg = NULL;
	size_t arg_len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "phpast", arg);

	RETURN_STR(strg);
}
/* }}} */

/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_phpast_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_phpast_init_globals(zend_phpast_globals *phpast_globals)
{
	phpast_globals->global_value = 0;
	phpast_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(phpast)
{
	ZVAL_UNDEF(&PHPAST_G(hook_callable));
	
	register_classes();

	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(phpast)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(phpast)
{
#if defined(COMPILE_DL_PHPAST) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(phpast)
{
	unhook_ast_process();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(phpast)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "phpast support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(arginfo_phpast___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phpast___destruct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phpast_eachChild, 0, 0, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phpast_getChildCount, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phpast_getChildren, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phpast_getKind, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phpast_getKindName, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phpast_isZval, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phpast_getZval, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phpast_setZval, 0, 0, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phpast___toString, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phpast___debugInfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phpast_enableAstHook, 0, 0, 1)
	ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phpast_disableAstHook, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phpast_compileFile, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phpast_compileString, 0, 0, 1)
	ZEND_ARG_INFO(0, code)
ZEND_END_ARG_INFO()


static const zend_function_entry phpast_methods[] = {
	PHP_ME(PHPAst, __construct, arginfo_phpast___construct, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	PHP_ME(PHPAst, __destruct, arginfo_phpast___destruct, ZEND_ACC_DTOR|ZEND_ACC_PUBLIC)
	PHP_ME(PHPAst, eachChild, arginfo_phpast_eachChild, ZEND_ACC_PUBLIC)
	PHP_ME(PHPAst, getChildCount, arginfo_phpast_getChildCount, ZEND_ACC_PUBLIC)
	PHP_ME(PHPAst, getChildren, arginfo_phpast_getChildren, ZEND_ACC_PUBLIC)
	PHP_ME(PHPAst, getKind, arginfo_phpast_getKind, ZEND_ACC_PUBLIC)
	PHP_ME(PHPAst, getKindName, arginfo_phpast_getKindName, ZEND_ACC_PUBLIC)
	PHP_ME(PHPAst, isZval, arginfo_phpast_isZval, ZEND_ACC_PUBLIC)
	PHP_ME(PHPAst, getZval, arginfo_phpast_getZval, ZEND_ACC_PUBLIC)
	PHP_ME(PHPAst, setZval, arginfo_phpast_setZval, ZEND_ACC_PUBLIC)
	PHP_ME(PHPAst, __toString, arginfo_phpast___toString, ZEND_ACC_PUBLIC)
	PHP_ME(PHPAst, __debugInfo, arginfo_phpast___debugInfo, ZEND_ACC_PUBLIC)
	PHP_ME(PHPAst, enableAstHook, arginfo_phpast_enableAstHook, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(PHPAst, disableAstHook, arginfo_phpast_disableAstHook, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(PHPAst, compileFile, arginfo_phpast_compileFile, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(PHPAst, compileString, arginfo_phpast_compileString, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};
/* }}} */

static void register_classes() { /* {{{ */
	zend_class_entry _ce_phpast;

	INIT_CLASS_ENTRY(_ce_phpast, "PHPAst", phpast_methods);
	ce_phpast = zend_register_internal_class(&_ce_phpast);
	ce_phpast->create_object = phpast_new;

	memcpy(&phpast_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	phpast_object_handlers.offset = XtOffsetOf(phpast_obj, std);
	phpast_object_handlers.free_obj = phpast_free;

	phpast_declare_kind_constant(ce_phpast);
}
/* }}} */

/* {{{ phpast_functions[]
 *
 * Every user visible function must have an entry in phpast_functions[].
 */
const zend_function_entry phpast_functions[] = {
	PHP_FE(confirm_phpast_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in phpast_functions[] */
};
/* }}} */

/* {{{ phpast_module_entry
 */
zend_module_entry phpast_module_entry = {
	STANDARD_MODULE_HEADER,
	"phpast",
	phpast_functions,
	PHP_MINIT(phpast),
	PHP_MSHUTDOWN(phpast),
	PHP_RINIT(phpast),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(phpast),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(phpast),
	PHP_PHPAST_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PHPAST
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE();
#endif
ZEND_GET_MODULE(phpast)
#endif

void hook_ast_process(zval *callable) /* {{{ */
{
	ZVAL_COPY(&PHPAST_G(hook_callable), callable);

	original_ast_process = zend_ast_process;
	zend_ast_process = php_ast_process;
}
/* }}} */

void unhook_ast_process() /* {{{ */
{
	zend_ast_process = original_ast_process;
	original_ast_process = NULL;

	zval_ptr_dtor(&PHPAST_G(hook_callable));
	ZVAL_UNDEF(&PHPAST_G(hook_callable));
}
/* }}} */

void php_ast_process(zend_ast *ast) { /* {{{ */
	zval args[1];
	zval retval;

	if (Z_ISUNDEF(PHPAST_G(hook_callable))) {
		return;
	}

//	php_printf("call php_ast_process\n");
	
	create_phpast_from_zend_ast(&args[0], ast);

	if (call_user_function(EG(function_table), NULL, 
		&PHPAST_G(hook_callable), &retval, 1, args) == SUCCESS &&  Z_TYPE(retval) != IS_UNDEF) {

		if (zend_is_true(&retval)) {
			if (Z_TYPE(retval) != IS_OBJECT || !instanceof_function(Z_OBJCE(retval), ce_phpast)) {
				zend_throw_error(NULL, "return value must be PHPAst Object");
			}

			CG(ast) = create_zend_ast_from_phpast(Z_PHPAST_P(&retval));
		}

		zval_ptr_dtor(&retval);
	}

	zval_ptr_dtor(&args[0]);

//	php_printf("end php_ast_process\n");
}
/* }}} */

PHP_METHOD(PHPAst, __construct) /* {{{ */
{
	zend_long kind;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_LONG(kind)
	ZEND_PARSE_PARAMETERS_END();
}
/* }}} */

PHP_METHOD(PHPAst, __destruct) /* {{{ */
{
}
/* }}} */

static int phpast_each_child(zval *zv, int num_args, va_list args, zend_hash_key *hash_key) /* {{{ */
{
	zend_fcall_info *fci = va_arg(args, zend_fcall_info*);
	zend_fcall_info_cache *fci_cache = va_arg(args, zend_fcall_info_cache*);
	phpast_obj *obj = Z_PHPAST_P(zv);
	zval retval, params[1];

	ZVAL_COPY(&params[0], zv);

	fci->param_count = 1;
	fci->params = params;
	fci->retval = &retval;
	fci->no_separation = 0;

	if (zend_call_function(fci, fci_cache) == SUCCESS && Z_TYPE(retval) != IS_UNDEF) {
		zval_ptr_dtor(&retval);
	}

	zval_ptr_dtor(&params[0]);
}
/* }}} */

PHP_METHOD(PHPAst, eachChild) /* {{{ */
{
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;
	phpast_obj *self = Z_PHPAST_P(getThis());

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_FUNC_EX(fci, fci_cache, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	zend_hash_apply_with_arguments(&self->children, phpast_each_child, 2, &fci, &fci_cache);
}
/* }}} */

PHP_METHOD(PHPAst, getChildCount) /* {{{ */
{
	phpast_obj *self = Z_PHPAST_P(getThis());

	RETURN_LONG(self->num_children);
}
/* }}} */

PHP_METHOD(PHPAst, getChildren) /* {{{ */
{
	phpast_obj *self = Z_PHPAST_P(getThis());

	RETURN_ARR(zend_array_dup(&self->children));
}
/* }}} */

PHP_METHOD(PHPAst, getKind) /* {{{ */
{
	phpast_obj *self = Z_PHPAST_P(getThis());

	RETURN_LONG(self->kind);
}
/* }}} */

PHP_METHOD(PHPAst, getKindName) /* {{{ */
{
	phpast_obj *self = Z_PHPAST_P(getThis());

	RETURN_STRING(phpast_get_kind_name(self->kind));
}
/* }}} */

PHP_METHOD(PHPAst, isZval) /* {{{ */
{
	phpast_obj *self = Z_PHPAST_P(getThis());

	if (self->kind == ZEND_AST_ZVAL) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */

PHP_METHOD(PHPAst, getZval) /* {{{ */
{
	phpast_obj *self = Z_PHPAST_P(getThis());

	if (self->kind == ZEND_AST_ZVAL) {
		RETURN_ZVAL(&self->val, 0, 0);
	}
}
/* }}} */

PHP_METHOD(PHPAst, setZval) /* {{{ */
{
	zval *zv;
	phpast_obj *self = Z_PHPAST_P(getThis());

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(zv)
	ZEND_PARSE_PARAMETERS_END();

	zval_ptr_dtor(&self->val);
	ZVAL_COPY(&self->val, zv);
}
/* }}} */

PHP_METHOD(PHPAst, __toString) /* {{{ */
{
	phpast_obj *self = Z_PHPAST_P(getThis());

	RETVAL_STRING(phpast_to_string(self));
}
/* }}} */

PHP_METHOD(PHPAst, __debugInfo) /* {{{ */
{
	zval tmp;
	zend_array* debug_info;
	phpast_obj *self = Z_PHPAST_P(getThis());

	ALLOC_HASHTABLE(debug_info);
	zend_hash_init(debug_info, 8, NULL, ZVAL_PTR_DTOR, 0);

	ZVAL_STRING(&tmp, phpast_get_kind_name(self->kind));
	zend_hash_str_update(debug_info, "kind", sizeof("kind")-1, &tmp);

	ZVAL_LONG(&tmp, self->attr);
	zend_hash_str_update(debug_info, "attr", sizeof("attr")-1, &tmp);

	ZVAL_LONG(&tmp, self->lineno);
	zend_hash_str_update(debug_info, "lineno", sizeof("lineno")-1, &tmp);

	ZVAL_LONG(&tmp, self->start_lineno);
	zend_hash_str_update(debug_info, "start_lineno", sizeof("start_lineno")-1, &tmp);

	ZVAL_LONG(&tmp, self->end_lineno);
	zend_hash_str_update(debug_info, "end_lineno", sizeof("end_lineno")-1, &tmp);

	ZVAL_LONG(&tmp, self->flags);
	zend_hash_str_update(debug_info, "flags", sizeof("flags")-1, &tmp);

	if (self->doc_comment) {
		ZVAL_STR(&tmp, self->doc_comment);
		zend_hash_str_update(debug_info, "doc_comment", sizeof("doc_comment")-1, &tmp);
	}

	if (self->name) {
		ZVAL_STR(&tmp, self->name);
		zend_hash_str_update(debug_info, "name", sizeof("name")-1, &tmp);
	}

	zend_hash_str_update(debug_info, "zval", sizeof("zval")-1, &self->val);

	ZVAL_STRING(&tmp, phpast_to_string(self));
	zend_hash_str_update(debug_info, "__toString", sizeof("__toString")-1, &tmp);

	ZVAL_ARR(&tmp, &self->children);
	zend_hash_str_update(debug_info, "children", sizeof("children")-1, &tmp);

	RETURN_ARR(debug_info);
}
/* }}} */

PHP_METHOD(PHPAst, enableAstHook) { /* {{{ */

	zval *callable;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL_EX(callable, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	if (!zend_is_callable(callable, 0, NULL)) {
		zend_throw_exception(spl_ce_InvalidArgumentException, 0, "Argument must be callable");
	}

	hook_ast_process(callable);
}
/* }}} */

PHP_METHOD(PHPAst, disableAstHook) /* {{{ */
{	
	unhook_ast_process();
}
/* }}} */

void _php_ast_process(zend_ast *ast) /* {{{ */
{
	create_phpast_from_zend_ast(&PHPAST_G(compiled_ast), ast);
}
/* }}} */

PHP_METHOD(PHPAst, compileFile) /* {{{ */
{	
	zend_string *filename;
	zval inc_file;
	zend_op_array *op_array;
	zend_ast_process_t original;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(filename)
	ZEND_PARSE_PARAMETERS_END();

	ZVAL_STR(&inc_file, filename);

	original = zend_ast_process;
	zend_ast_process = _php_ast_process;
	op_array = compile_filename(ZEND_INCLUDE, &inc_file);
	zend_ast_process = original;

	if (UNEXPECTED(op_array == NULL)) {
		RETURN_NULL();
	}

	destroy_op_array(op_array);
	efree(op_array);

	RETURN_ZVAL(&PHPAST_G(compiled_ast), 0, 0);
}
/* }}} */

PHP_METHOD(PHPAst, compileString) /* {{{ */
{	
	zend_string *code;
	zend_op_array *op_array;
	zend_ast_process_t original;
	zval source_string;
	char* eval_desc;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(code)
	ZEND_PARSE_PARAMETERS_END();

	ZVAL_STR(&source_string, code);
    eval_desc = zend_make_compiled_string_description("PHPAst::compileString");

	original = zend_ast_process;
	zend_ast_process = _php_ast_process;
	op_array = compile_string(&source_string, eval_desc);
	zend_ast_process = original;

	efree(eval_desc);
	if (UNEXPECTED(op_array == NULL)) {
		RETURN_NULL();
	}

	destroy_op_array(op_array);
	efree(op_array);

	RETURN_ZVAL(&PHPAST_G(compiled_ast), 0, 0);
}
/* }}} */

static zend_bool ast_kind_is_decl(zend_ast_kind kind) /* {{{ */
{
    return (
        kind == ZEND_AST_FUNC_DECL ||
        kind == ZEND_AST_CLOSURE ||
        kind == ZEND_AST_METHOD ||
        kind == ZEND_AST_CLASS
    );
}
/* }}} */

static zend_bool ast_kind_is_list(zend_ast_kind kind) /* {{{ */
{
	// see: zend_ast_is_list
	return (kind >> ZEND_AST_IS_LIST_SHIFT) & 1;
}
/* }}} */

static uint32_t ast_kind_get_num_children(zend_ast_kind kind) /* {{{ */
{
	// see: zend_ast_get_num_children
	ZEND_ASSERT(!ast_kind_is_list(kind));
	return kind >> ZEND_AST_NUM_CHILDREN_SHIFT;
}
/* }}} */

static void ast_destroy(zend_ast *ast) { /* {{{ */
	if (ast == NULL) {
		return;
	}
	
	if (ast->kind == ZEND_AST_ZVAL) {
		zval_ptr_dtor_nogc(zend_ast_get_zval(ast));
	}
	efree(ast);
}
/* }}} */

static zend_object *phpast_new(zend_class_entry *class_type) /* {{{ */
{
	phpast_obj *self;
	self = ecalloc(1, sizeof(phpast_obj) + zend_object_properties_size(class_type));

	zend_object_std_init(&self->std, class_type);
	object_properties_init(&self->std, class_type);
	self->std.handlers = &phpast_object_handlers;

	zend_hash_init(&self->children, HT_MIN_SIZE, NULL, NULL, 0);

	return &self->std;
}
/* }}} */

static void phpast_free(zend_object *object) /* {{{ */
{
	phpast_obj *self = phpast_obj_from_obj(object);

	if (self->doc_comment) {
		zend_string_release(self->doc_comment);
	}

	if (self->name) {
		zend_string_release(self->name);
	}

	if (self->kind == ZEND_AST_ZVAL) {
		zval_ptr_dtor_nogc(&self->val);
	}

	zend_hash_destroy(&self->children);

	zend_object_std_dtor(&self->std);

	efree(self);
}
/* }}} */

static void create_phpast_from_zend_ast(zval *obj, zend_ast *ast) /* {{{ */
{
	int i;
	zend_ast **ast_children;
	phpast_obj *self;

	object_init_ex(obj, ce_phpast);
	self = Z_PHPAST_P(obj);

	ZEND_ASSERT(ast);
	if (ast->kind == ZEND_AST_ZVAL) {
		zend_ast_zval *zv = (zend_ast_zval*) ast;
		self->kind = ZEND_AST_ZVAL;
		self->attr = zv->attr;
		self->lineno = zv->val.u2.lineno;
		ZVAL_COPY(&self->val, &zv->val);
		return;
	}
	
	if (ast_kind_is_decl(ast->kind)) {
		zend_ast_decl *decl = (zend_ast_decl *) ast;
		self->kind = decl->kind;
		self->attr = decl->attr;
		self->start_lineno = decl->start_lineno;
		self->end_lineno = decl->end_lineno;
		self->flags = decl->flags;
		if (decl->doc_comment) {
			self->doc_comment = zend_string_copy(decl->doc_comment);
		}
		else {
			self->doc_comment = NULL;
		}
		self->name = zend_string_copy(decl->name);

		self->num_children = 4;
		ast_children = decl->child;
	} else if (ast_kind_is_list(ast->kind)) {
		zend_ast_list *list = zend_ast_get_list(ast);
		self->kind = list->kind;
		self->attr = list->attr;
		self->lineno = list->lineno;

		self->num_children = list->children;
		ast_children = list->child;
	} else {
		self->kind = ast->kind;
		self->attr = ast->attr;
		self->lineno = ast->lineno;

		self->num_children = zend_ast_get_num_children(ast);
		ast_children = ast->child;
	}

	for(i=0; i<self->num_children; ++i) {
		zval child_phpast;
		if (ast_children[i]) {
			create_phpast_from_zend_ast(&child_phpast, ast_children[i]);
		} else {
			ZVAL_NULL(&child_phpast);
		}
		zend_hash_next_index_insert(&self->children, &child_phpast);
	}
}
/* }}} */

static zend_ast *create_zend_ast_from_phpast(phpast_obj *obj) { /* {{{ */

	zend_ast *new;

	if (obj->kind == ZEND_AST_ZVAL) {
		return zend_ast_create_zval_ex(&obj->val, obj->attr);
	} else if (ast_kind_is_decl(obj->kind)) {
		zend_ast *child1, *child2, *child3, *child4;
		HashPosition pos = 0;

		child1 = create_zend_ast_from_phpast(Z_PHPAST_P(zend_hash_get_current_data_ex(&obj->children, &pos))); ++pos;
		child2 = create_zend_ast_from_phpast(Z_PHPAST_P(zend_hash_get_current_data_ex(&obj->children, &pos))); ++pos;
		child3 = create_zend_ast_from_phpast(Z_PHPAST_P(zend_hash_get_current_data_ex(&obj->children, &pos))); ++pos;
		child4 = create_zend_ast_from_phpast(Z_PHPAST_P(zend_hash_get_current_data_ex(&obj->children, &pos)));

		return zend_ast_create_decl(
			obj->kind,
			obj->flags,
			obj->start_lineno,
			zend_string_copy(obj->doc_comment),
			zend_string_copy(obj->name),
			child1, child2, child3, child4
		);
	}
	
	if (ast_kind_is_list(obj->kind)) {
		zval *child;

		new = zend_ast_create_list(0, obj->kind);
		new->lineno = obj->lineno;

		ZEND_HASH_FOREACH_VAL(&obj->children, child) {
			zend_ast_list_add(new, create_zend_ast_from_phpast(Z_PHPAST_P(child)));
		} ZEND_HASH_FOREACH_END();

		return new;
	} else {
		uint32_t child_num = ast_kind_get_num_children(obj->kind);
		HashPosition pos = 0;
		zend_ast *child1, *child2, *child3, *child4;
		
		if (child_num == 0) {
			return zend_ast_create_ex(obj->kind, obj->attr);
		}

		child1 = create_zend_ast_from_phpast(Z_PHPAST_P(zend_hash_get_current_data_ex(&obj->children, &pos)));
		if (child_num == 1) {
			return zend_ast_create_ex(obj->kind, obj->attr, child1);
		}

		++pos;
		child2 = create_zend_ast_from_phpast(Z_PHPAST_P(zend_hash_get_current_data_ex(&obj->children, &pos)));
		if (child_num == 2) {
			return zend_ast_create_ex(obj->kind, obj->attr, child1, child2);
		}

		++pos;
		child3 = create_zend_ast_from_phpast(Z_PHPAST_P(zend_hash_get_current_data_ex(&obj->children, &pos)));
		if (child_num == 3) {
			return zend_ast_create_ex(obj->kind, obj->attr, child1, child2, child3);
		}

		++pos;
		child4 = create_zend_ast_from_phpast(Z_PHPAST_P(zend_hash_get_current_data_ex(&obj->children, &pos)));
		if (child_num == 4) {
			return zend_ast_create_ex(obj->kind, obj->attr, child1, child2, child3, child4);
		}
	}

}
/* }}} */

static char *phpast_zval_to_string(zval *zv) /* {{{ */
{
	ZVAL_DEREF(zv);
	switch (Z_TYPE_P(zv)) {
		case IS_NULL:	return "NULL";
		case IS_FALSE:	return "false";
		case IS_TRUE:	return "true";
		case IS_LONG:	return "(long)";
		case IS_DOUBLE:	return "(float)";
		case IS_STRING:	return "(string)";
		case IS_ARRAY:	return "(array)";
		case IS_CONSTANT:	return "(const)";
		case IS_CONSTANT_AST:	return "(const ast)";
	}

	ZEND_ASSERT(0);
	return "";
}
/* }}} */

static char *phpast_to_string(phpast_obj *obj) /* {{{ */
{
	switch (obj->kind) {
		case ZEND_AST_ZVAL:
			return phpast_zval_to_string(&obj->val);
		case ZEND_AST_CLASS:
			if (obj->flags & ZEND_ACC_INTERFACE) {
				return "interface";
			} else if (obj->flags & ZEND_ACC_TRAIT) {
				return "trait";
			} else {
				if (obj->flags & ZEND_ACC_EXPLICIT_ABSTRACT_CLASS) {
					return "abstract class";
				} else if (obj->flags & ZEND_ACC_FINAL) {
					return "final class";
				} else if (obj->flags & ZEND_ACC_ANON_CLASS) {
					return "(anonymous) class";
				} else {
					return "class";
				}
			}

		case ZEND_AST_PROP_DECL:
			if (obj->attr & ZEND_ACC_STATIC) {
				if (obj->attr & ZEND_ACC_PUBLIC)    { return "public static"; }
				if (obj->attr & ZEND_ACC_PROTECTED) { return "protected static"; }
				if (obj->attr & ZEND_ACC_PRIVATE)   { return "private static"; }
			} else {
				if (obj->attr & ZEND_ACC_PUBLIC)    { return "public"; }
				if (obj->attr & ZEND_ACC_PROTECTED) { return "protected"; }
				if (obj->attr & ZEND_ACC_PRIVATE)   { return "private"; }
			}
			break;
		case ZEND_AST_USE:
			switch (obj->attr) {
				case T_FUNCTION: return "function";
				case T_CONST:    return "const";
				case T_CLASS:    return "class";
			}
			break;
		case ZEND_AST_MAGIC_CONST:
			switch(obj->attr) {
				case T_LINE:     return "__LINE__";
				case T_FILE:     return "__FILE__";
				case T_DIR:      return "__DIR__";
				case T_TRAIT_C:  return "__TRAIT__";
				case T_METHOD_C: return "__METHOD__";
				case T_FUNC_C:   return "__FUNCTION__";
				case T_NS_C:     return "__NAMESPACE__";
				case T_CLASS_C:  return "__CLASS__";
				EMPTY_SWITCH_DEFAULT_CASE();
			}
		case ZEND_AST_TYPE:
			switch (obj->attr) {
				case IS_ARRAY:    return "array";
				case IS_CALLABLE: return "callable";
				EMPTY_SWITCH_DEFAULT_CASE();
			}
		case ZEND_AST_CAST:
			switch(obj->attr) {
				case IS_NULL:    return "(unset)";
				case _IS_BOOL:   return "(bool)";
				case IS_LONG:    return "(int)";
				case IS_DOUBLE:  return "(float)";
				case IS_STRING:  return "(string)";
				case IS_ARRAY:   return "(array)";
				case IS_OBJECT:  return "(selfect)";
				EMPTY_SWITCH_DEFAULT_CASE();
			}
		case ZEND_AST_UNARY_OP:
			switch(obj->attr) {
				case ZEND_BW_NOT:   return "~";
				case ZEND_BOOL_NOT: return "!";
				EMPTY_SWITCH_DEFAULT_CASE();
			}
		case ZEND_AST_ASSIGN_OP:
			switch(obj->attr) {
				case ZEND_ASSIGN_ADD:    return "+=";
				case ZEND_ASSIGN_SUB:    return "-=";
				case ZEND_ASSIGN_MUL:    return "*=";
				case ZEND_ASSIGN_DIV:    return "/=";
				case ZEND_ASSIGN_MOD:    return "%=";
				case ZEND_ASSIGN_SL:     return "<<=";
				case ZEND_ASSIGN_SR:     return ">>=";
				case ZEND_ASSIGN_CONCAT: return ".=";
				case ZEND_ASSIGN_BW_OR:  return "|=";
				case ZEND_ASSIGN_BW_AND: return "&=";
				case ZEND_ASSIGN_BW_XOR: return "^=";
				case ZEND_POW:           return "**=";
				EMPTY_SWITCH_DEFAULT_CASE();
			}
		case ZEND_AST_BINARY_OP:
			switch(obj->attr) {
				case ZEND_ADD:                 return "+";
				case ZEND_SUB:                 return "-";
				case ZEND_MUL:                 return "*";
				case ZEND_DIV:                 return "/";
				case ZEND_MOD:                 return "%";
				case ZEND_SL:                  return "<<";
				case ZEND_SR:                  return ">>";
				case ZEND_CONCAT:              return ".";
				case ZEND_BW_OR:               return "|";
				case ZEND_BW_AND:              return "&";
				case ZEND_BW_XOR:              return "^";
				case ZEND_IS_IDENTICAL:        return "===";
				case ZEND_IS_NOT_IDENTICAL:    return "!==";
				case ZEND_IS_EQUAL:            return "==";
				case ZEND_IS_NOT_EQUAL:        return "!=";
				case ZEND_IS_SMALLER:          return "<";
				case ZEND_IS_SMALLER_OR_EQUAL: return "<=";
				case ZEND_POW:                 return "**";
				case ZEND_BOOL_XOR:            return "xor";
				case ZEND_SPACESHIP:           return "<=>";
				EMPTY_SWITCH_DEFAULT_CASE();
			}
	}
	return "";
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

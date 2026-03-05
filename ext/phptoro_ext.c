/*
 * phptoro_ext.c — PHP built-in function bridge for phpToro.
 *
 * Exposes phptoro(string $ns): PhpToroPlugin
 * PhpToroPlugin::__call() dispatches "ns.method" to the native side,
 * with automatic JSON encode/decode of arguments and return values.
 *
 * The host calls phptoro_ext_prepare() before sapi_startup(), then passes
 * &phptoro_module_entry to php_module_startup() so the class is registered
 * during MINIT (required by zend_register_internal_class).
 */

#include "php.h"
#include "main/SAPI.h"
#include "php_json.h"
#include "zend_smart_str.h"

/* Native dispatch callback — set via phptoro_ext_prepare() */
static char *(*g_call_fn)(const char *cmd, const char *json) = NULL;

/* ── PhpToroPlugin class ───────────────────────────────────────────────── */

typedef struct {
    zend_string *ns;
    zend_object std;
} phptoro_plugin_obj;

static zend_class_entry *phptoro_plugin_ce = NULL;
static zend_object_handlers phptoro_plugin_handlers;

static inline phptoro_plugin_obj *phptoro_plugin_from_obj(zend_object *obj) {
    return (phptoro_plugin_obj *)((char *)obj - XtOffsetOf(phptoro_plugin_obj, std));
}

static zend_object *phptoro_plugin_create(zend_class_entry *ce) {
    phptoro_plugin_obj *intern = zend_object_alloc(sizeof(phptoro_plugin_obj), ce);
    intern->ns = NULL;
    zend_object_std_init(&intern->std, ce);
    object_properties_init(&intern->std, ce);
    intern->std.handlers = &phptoro_plugin_handlers;
    return &intern->std;
}

static void phptoro_plugin_free(zend_object *obj) {
    phptoro_plugin_obj *intern = phptoro_plugin_from_obj(obj);
    if (intern->ns) {
        zend_string_release(intern->ns);
    }
    zend_object_std_dtor(&intern->std);
}

/* ── __call(string $method, array $args): mixed ────────────────────────── */

ZEND_BEGIN_ARG_INFO_EX(arginfo_phptoro_plugin___call, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, method, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, args, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(PhpToroPlugin, __call)
{
    zend_string *method;
    zval *args;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(method)
        Z_PARAM_ZVAL(args)
    ZEND_PARSE_PARAMETERS_END();

    if (!g_call_fn) {
        php_error_docref(NULL, E_WARNING, "phptoro: bridge not initialised");
        RETURN_FALSE;
    }

    phptoro_plugin_obj *intern = phptoro_plugin_from_obj(Z_OBJ_P(ZEND_THIS));

    /* Build "ns.method" command */
    zend_string *command = zend_strpprintf(0, "%s.%s",
        ZSTR_VAL(intern->ns), ZSTR_VAL(method));

    /* JSON-encode args for the native side.
     * 0 args  → "{}"           e.g. ->read()
     * 1 arg   → that arg       e.g. ->send(['title'=>'Hi']) → {"title":"Hi"}
     * N args  → full array     e.g. ->set('key','val') → ["key","val"]  */
    HashTable *ht = Z_ARRVAL_P(args);
    uint32_t argc = zend_hash_num_elements(ht);

    smart_str buf = {0};
    if (argc == 0) {
        smart_str_appends(&buf, "{}");
    } else if (argc == 1) {
        zval *first = zend_hash_index_find(ht, 0);
        if (first) {
            php_json_encode(&buf, first, 0);
        } else {
            php_json_encode(&buf, args, 0);
        }
    } else {
        php_json_encode(&buf, args, 0);
    }
    smart_str_0(&buf);

    /* Call native dispatcher */
    char *result = g_call_fn(ZSTR_VAL(command),
        buf.s ? ZSTR_VAL(buf.s) : "{}");

    zend_string_release(command);
    smart_str_free(&buf);

    if (!result) {
        RETURN_FALSE;
    }

    /* JSON-decode result into return_value (assoc arrays) */
    php_json_decode(return_value, result, strlen(result), 1,
        PHP_JSON_PARSER_DEFAULT_DEPTH);
    free(result);
}

static const zend_function_entry phptoro_plugin_methods[] = {
    PHP_ME(PhpToroPlugin, __call, arginfo_phptoro_plugin___call, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

/* ── phptoro() function ────────────────────────────────────────────────── */

ZEND_BEGIN_ARG_INFO_EX(arginfo_phptoro, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, namespace, IS_STRING, 0)
ZEND_END_ARG_INFO()

PHP_FUNCTION(phptoro)
{
    zend_string *ns;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(ns)
    ZEND_PARSE_PARAMETERS_END();

    object_init_ex(return_value, phptoro_plugin_ce);
    phptoro_plugin_obj *intern = phptoro_plugin_from_obj(Z_OBJ_P(return_value));
    intern->ns = zend_string_copy(ns);
}

/* ── Function table ────────────────────────────────────────────────────── */

static const zend_function_entry phptoro_fn_table[] = {
    PHP_FE(phptoro, arginfo_phptoro)
    PHP_FE_END
};

/* ── Module entry (MINIT registers the class) ──────────────────────────── */

static PHP_MINIT_FUNCTION(phptoro)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "PhpToroPlugin", phptoro_plugin_methods);
    phptoro_plugin_ce = zend_register_internal_class(&ce);
    phptoro_plugin_ce->create_object = phptoro_plugin_create;

    memcpy(&phptoro_plugin_handlers, &std_object_handlers,
        sizeof(zend_object_handlers));
    phptoro_plugin_handlers.offset = XtOffsetOf(phptoro_plugin_obj, std);
    phptoro_plugin_handlers.free_obj = phptoro_plugin_free;

    return SUCCESS;
}

zend_module_entry phptoro_module_entry = {
    STANDARD_MODULE_HEADER,
    "phptoro",              /* name */
    phptoro_fn_table,       /* functions */
    PHP_MINIT(phptoro),     /* MINIT */
    NULL,                   /* MSHUTDOWN */
    NULL,                   /* RINIT */
    NULL,                   /* RSHUTDOWN */
    NULL,                   /* MINFO */
    "0.1",                  /* version */
    STANDARD_MODULE_PROPERTIES
};

/* ── Public API ────────────────────────────────────────────────────────── */

const zend_function_entry *phptoro_ext_functions(void) {
    return phptoro_fn_table;
}

void phptoro_ext_prepare(char *(*call_fn)(const char *, const char *)) {
    g_call_fn = call_fn;
}

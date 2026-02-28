/*
 * phptoro_ext.c — PHP built-in function bridge for phpToro.
 *
 * Exposes phptoro() registered via sapi_module.additional_functions.
 *
 *   phptoro(string $command, string $params_json = '{}'): string|false
 *
 * Rust calls phptoro_ext_prepare() before RiphtSapi::instance().
 */

#include "php.h"
#include "main/SAPI.h"

/* Rust dispatch callback — set via phptoro_ext_prepare() */
static char *(*g_call_fn)(const char *cmd, const char *json) = NULL;

/* ── Arg info ────────────────────────────────────────────────────────────── */

ZEND_BEGIN_ARG_INFO_EX(arginfo_phptoro, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, command,     IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, params_json, IS_STRING, 1)
ZEND_END_ARG_INFO()

/* ── PHP function forward declarations ───────────────────────────────────── */

PHP_FUNCTION(phptoro);

/* ── Function table ──────────────────────────────────────────────────────── */

static const zend_function_entry phptoro_fn_table[] = {
    PHP_FE(phptoro,  arginfo_phptoro)
    PHP_FE_END
};

/* ── Public API ──────────────────────────────────────────────────────────── */

const zend_function_entry *phptoro_ext_functions(void) {
    return phptoro_fn_table;
}

/*
 * Custom INI entries for embedded PHP.
 */
static const char phptoro_ini_entries[] =
    "variables_order=EGPCS\n"
    "request_order=GP\n"
    "output_buffering=4096\n"
    "implicit_flush=0\n"
    "html_errors=0\n"
    "display_errors=1\n"
    "log_errors=1\n"
    "opcache.enable=0\n"
    "opcache.enable_cli=0\n"
    "\0";

static void phptoro_ini_defaults(HashTable *ht) {
    sapi_module.ini_entries = phptoro_ini_entries;
}

void phptoro_ext_prepare(char *(*call_fn)(const char *, const char *)) {
    g_call_fn = call_fn;
    sapi_module.additional_functions = phptoro_fn_table;
    sapi_module.ini_defaults = phptoro_ini_defaults;
}

/* ── PHP function implementations ────────────────────────────────────────── */

/*
 * phptoro(string $command, ?string $params_json = '{}'): string|false
 *
 * Calls a native command synchronously.
 * Returns the JSON response string, or false on error.
 *
 * Example:
 *   $result = phptoro('notification.show', json_encode(['title' => 'Hi']));
 */
PHP_FUNCTION(phptoro)
{
    char   *command     = NULL;
    size_t  command_len = 0;
    char   *params      = NULL;
    size_t  params_len  = 0;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STRING(command, command_len)
        Z_PARAM_OPTIONAL
        Z_PARAM_STRING(params, params_len)
    ZEND_PARSE_PARAMETERS_END();

    if (!g_call_fn) {
        php_error_docref(NULL, E_WARNING, "phptoro: bridge not initialised");
        RETURN_FALSE;
    }

    const char *json = (params && params_len > 0) ? params : "{}";
    char *result = g_call_fn(command, json);

    if (!result) {
        RETURN_FALSE;
    }

    RETVAL_STRING(result);
    free(result);
}

#ifndef PHPTORO_EXT_H
#define PHPTORO_EXT_H

#include "php.h"

/* Callback type: native dispatcher function called by PhpToroPlugin::__call() */
typedef char *(*phptoro_call_fn_t)(const char *command, const char *json_args);

/*
 * phptoro_ext_prepare() — call BEFORE sapi_startup().
 *
 * Stores the native dispatch callback used by PhpToroPlugin::__call().
 */
void phptoro_ext_prepare(phptoro_call_fn_t call_fn);

/* Module entry — pass to php_module_startup() to register PhpToroPlugin class
 * and the phptoro() function during MINIT. */
extern zend_module_entry phptoro_module_entry;

/* Returns the zend_function_entry table (for legacy use). */
const zend_function_entry *phptoro_ext_functions(void);

#endif /* PHPTORO_EXT_H */

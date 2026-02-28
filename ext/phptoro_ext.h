#ifndef PHPTORO_EXT_H
#define PHPTORO_EXT_H

#include "php.h"

/* Callback type: Rust dispatcher function called by phptoro_call() */
typedef char *(*phptoro_call_fn_t)(const char *command, const char *json_args);

/*
 * phptoro_ext_prepare() — call BEFORE RiphtSapi::instance().
 *
 * Sets sapi_module.additional_functions so PHP registers phptoro_call()
 * automatically during php_module_startup().
 */
void phptoro_ext_prepare(phptoro_call_fn_t call_fn);

/* Returns the zend_function_entry table. */
const zend_function_entry *phptoro_ext_functions(void);

#endif /* PHPTORO_EXT_H */

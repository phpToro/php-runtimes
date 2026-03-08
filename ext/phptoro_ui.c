#include "phptoro_ui.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Directive buffer — collects directives during a single PHP request.
 * phptoro_ui_flush() returns them as JSON and resets the buffer.
 */

#define MAX_DIRECTIVES 64
#define MAX_DIRECTIVE_LEN 4096

static char *directives[MAX_DIRECTIVES];
static int directive_count = 0;

static void add_directive(const char *json) {
    if (directive_count >= MAX_DIRECTIVES) return;
    directives[directive_count++] = strdup(json);
}

/*
 * Plugin handler — receives command + JSON args from PHP.
 *
 * phptoro('ui')->alert('Title', 'Message')
 *   → command="alert", json='["Title","Message"]'
 *
 * phptoro('ui')->navigate('HomeScreen', ['id' => 1])
 *   → command="navigate", json='["HomeScreen",{"id":1}]'
 */
static char *ui_handle(const char *command, const char *json) {
    /* Build a directive JSON object: {"type":"command", "args": json} */
    size_t cmd_len = strlen(command);
    size_t json_len = json ? strlen(json) : 2;
    const char *json_val = (json && strlen(json) > 0) ? json : "[]";

    size_t buf_len = cmd_len + json_len + 64;
    char *buf = malloc(buf_len);
    if (!buf) return strdup("{\"ok\":true}");

    snprintf(buf, buf_len, "{\"type\":\"%s\",\"args\":%s}", command, json_val);
    add_directive(buf);
    free(buf);

    return strdup("{\"ok\":true}");
}

void phptoro_ui_register(void) {
    phptoro_plugin plugin;
    plugin.ns = "ui";
    plugin.handle = ui_handle;
    phptoro_register_plugin(plugin);
}

char *phptoro_ui_flush(void) {
    if (directive_count == 0) {
        return strdup("[]");
    }

    /* Calculate total size */
    size_t total = 2; /* [] */
    for (int i = 0; i < directive_count; i++) {
        total += strlen(directives[i]) + 1; /* +1 for comma */
    }

    char *result = malloc(total + 1);
    if (!result) {
        /* Clean up and return empty */
        for (int i = 0; i < directive_count; i++) free(directives[i]);
        directive_count = 0;
        return strdup("[]");
    }

    char *p = result;
    *p++ = '[';
    for (int i = 0; i < directive_count; i++) {
        if (i > 0) *p++ = ',';
        size_t len = strlen(directives[i]);
        memcpy(p, directives[i], len);
        p += len;
        free(directives[i]);
    }
    *p++ = ']';
    *p = '\0';

    directive_count = 0;
    return result;
}

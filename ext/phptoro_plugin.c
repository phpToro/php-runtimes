#include "phptoro_plugin.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static phptoro_plugin plugins[PHPTORO_MAX_PLUGINS];
static int plugin_count = 0;

void phptoro_register_plugin(phptoro_plugin plugin) {
    if (plugin_count < PHPTORO_MAX_PLUGINS) {
        plugins[plugin_count++] = plugin;
    }
}

char *phptoro_dispatch(const char *full_command, const char *json) {
    if (!full_command) {
        return strdup("{\"error\":\"null command\"}");
    }

    /* Split "namespace.command" on first dot */
    const char *dot = strchr(full_command, '.');
    if (!dot) {
        /* No dot — treat entire string as namespace, empty command */
        for (int i = 0; i < plugin_count; i++) {
            if (strcmp(plugins[i].ns, full_command) == 0) {
                return plugins[i].handle("", json ? json : "{}");
            }
        }
    } else {
        size_t ns_len = dot - full_command;
        const char *command = dot + 1;

        for (int i = 0; i < plugin_count; i++) {
            if (strlen(plugins[i].ns) == ns_len &&
                strncmp(plugins[i].ns, full_command, ns_len) == 0) {
                return plugins[i].handle(command, json ? json : "{}");
            }
        }
    }

    /* No plugin matched */
    char *err = malloc(256);
    snprintf(err, 256, "{\"error\":\"unknown command: %s\"}", full_command);
    return err;
}

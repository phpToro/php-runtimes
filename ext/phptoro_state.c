#include "phptoro_state.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

/*
 * State storage — one cJSON object per screen.
 * Simple linear scan — apps have few screens active at once.
 */

#define MAX_SCREENS 32

typedef struct {
    char *name;
    cJSON *state;
} screen_state;

static screen_state screens[MAX_SCREENS];
static int screen_count = 0;
static char *active_screen = NULL;

static cJSON *find_or_create(const char *screen) {
    for (int i = 0; i < screen_count; i++) {
        if (strcmp(screens[i].name, screen) == 0) {
            return screens[i].state;
        }
    }
    if (screen_count >= MAX_SCREENS) return NULL;
    screens[screen_count].name = strdup(screen);
    screens[screen_count].state = cJSON_CreateObject();
    return screens[screen_count++].state;
}

/*
 * Plugin handler — receives command + JSON args from PHP.
 *
 * phptoro('state')->get('email')
 *   → command="get", json='"email"'  (1 arg = sent directly)
 *
 * phptoro('state')->get('email', '')
 *   → command="get", json='["email",""]'  (2 args = array)
 *
 * phptoro('state')->set('email', 'user@test.com')
 *   → command="set", json='["email","user@test.com"]'
 */
static char *state_handle(const char *command, const char *json) {
    if (!active_screen) return strdup("null");

    cJSON *state = find_or_create(active_screen);
    if (!state) return strdup("null");

    cJSON *args = cJSON_Parse(json);
    if (!args) return strdup("null");

    if (strcmp(command, "get") == 0) {
        /* get('key') or get('key', default) */
        const char *key = NULL;
        cJSON *default_val = NULL;

        if (cJSON_IsArray(args)) {
            cJSON *first = cJSON_GetArrayItem(args, 0);
            key = cJSON_GetStringValue(first);
            default_val = cJSON_GetArrayItem(args, 1);
        } else if (cJSON_IsString(args)) {
            key = cJSON_GetStringValue(args);
        }

        if (!key) { cJSON_Delete(args); return strdup("null"); }

        cJSON *val = cJSON_GetObjectItemCaseSensitive(state, key);
        char *result;
        if (val) {
            result = cJSON_PrintUnformatted(val);
        } else if (default_val) {
            result = cJSON_PrintUnformatted(default_val);
        } else {
            result = strdup("null");
        }
        cJSON_Delete(args);
        return result;

    } else if (strcmp(command, "set") == 0) {
        /* set('key', value) */
        if (!cJSON_IsArray(args) || cJSON_GetArraySize(args) < 2) {
            cJSON_Delete(args);
            return strdup("{\"ok\":true}");
        }
        const char *key = cJSON_GetStringValue(cJSON_GetArrayItem(args, 0));
        cJSON *val = cJSON_GetArrayItem(args, 1);
        if (key && val) {
            cJSON_DeleteItemFromObjectCaseSensitive(state, key);
            cJSON_AddItemToObject(state, key, cJSON_Duplicate(val, 1));
        }
        cJSON_Delete(args);
        return strdup("{\"ok\":true}");

    } else if (strcmp(command, "all") == 0) {
        /* all() — return full state object */
        cJSON_Delete(args);
        char *result = cJSON_PrintUnformatted(state);
        return result ? result : strdup("{}");
    }

    cJSON_Delete(args);
    return strdup("null");
}

void phptoro_state_register(void) {
    phptoro_plugin plugin;
    plugin.ns = "state";
    plugin.handle = state_handle;
    phptoro_register_plugin(plugin);
}

void phptoro_state_set_screen(const char *screen) {
    free(active_screen);
    active_screen = screen ? strdup(screen) : NULL;
}

void phptoro_state_set(const char *screen, const char *key, const char *json_value) {
    cJSON *state = find_or_create(screen);
    if (!state) return;

    cJSON *val = cJSON_Parse(json_value);
    if (!val) return;

    cJSON_DeleteItemFromObjectCaseSensitive(state, key);
    cJSON_AddItemToObject(state, key, val);
}

char *phptoro_state_get(const char *screen) {
    for (int i = 0; i < screen_count; i++) {
        if (strcmp(screens[i].name, screen) == 0) {
            char *result = cJSON_PrintUnformatted(screens[i].state);
            return result ? result : strdup("{}");
        }
    }
    return strdup("{}");
}

void phptoro_state_clear(const char *screen) {
    for (int i = 0; i < screen_count; i++) {
        if (strcmp(screens[i].name, screen) == 0) {
            cJSON_Delete(screens[i].state);
            screens[i].state = cJSON_CreateObject();
            return;
        }
    }
}

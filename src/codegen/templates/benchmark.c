#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CONFIG_PATH "../config/config.ini"

// Global configuration variables
char* EXPERIMENT_VERSION = NULL;
int EXPERIMENT_LOOP_COUNT = 0;
int EXPERIMENT_ITERATIONS = 0;

// Function prototypes
char* trim_whitespace(char* str);
void load_config(const char* filename);
char* get_config_string(const char* key);
int get_config_int(const char* key);
int get_config_bool(const char* key);

// A simple structure to hold key-value pairs
typedef struct {
    char key[256];
    char value[256];
} config_entry;

// Assuming a small-medium number of configurations
config_entry config[256];
int config_size = 0;

int main() {
    load_config(CONFIG_PATH);
    
    EXPERIMENT_VERSION = get_config_string("experiment_version");
    EXPERIMENT_LOOP_COUNT = get_config_int("experiment_loop_count");
    EXPERIMENT_ITERATIONS = get_config_int("experiment_iterations");
    
    printf("Experiment Version: %s\n", EXPERIMENT_VERSION);
    printf("Iterations: %d\n", EXPERIMENT_ITERATIONS);
    
    // Your benchmark code goes here
    
    free(EXPERIMENT_VERSION);
    return 0;
}

char* trim_whitespace(char* str) {
    char* end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

void load_config(const char* filename) {
    char line[512];
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Unable to open the config file.\n");
        exit(1);
    }

    while (fgets(line, sizeof(line), file)) {
        char* key = strtok(line, "=");
        char* value = strtok(NULL, "\n");

        if (key && value) {
            strncpy(config[config_size].key, trim_whitespace(key), sizeof(config[config_size].key) - 1);
            strncpy(config[config_size].value, trim_whitespace(value), sizeof(config[config_size].value) - 1);
            config[config_size].key[sizeof(config[config_size].key) - 1] = '\0'; // Ensure null-termination
            config[config_size].value[sizeof(config[config_size].value) - 1] = '\0'; // Ensure null-termination

            config_size++;
        }
    }

    fclose(file);
}

char* get_config_string(const char* key) {
    for (int i = 0; i < config_size; i++) {
        if (strcmp(config[i].key, key) == 0) {
            return strdup(config[i].value);
        }
    }
    return NULL; // Key not found
}

int get_config_int(const char* key) {
    char* val_str = get_config_string(key);
    if (val_str) {
        int val = atoi(val_str);
        free(val_str);
        return val;
    }
    return 0; // Key not found or conversion error
}

int get_config_bool(const char* key) {
    char* val_str = get_config_string(key);
    if (val_str) {
        int val = (strcmp(val_str, "true") == 0 || strcmp(val_str, "1") == 0);
        free(val_str);
        return val;
    }
    return 0; // Key not found or conversion error
}


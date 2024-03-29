#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#define CONFIG_PATH "../config/config.ini"

// Global configuration variables
char* EXPERIMENT_VERSION = NULL;
int   EXPERIMENT_LOOP_COUNT = 0;
int   EXPERIMENT_ITERATIONS = 0;
int   EXPERIMENT_RUN_ID = 0;
char* EXPERIMENT_CONFIGURATION_NAME = NULL;

// Function prototypes
char* trim_whitespace(char* str);
void load_config(const char* filename);
char* get_config_string(const char* key);
int get_config_int(const char* key);
int get_config_bool(const char* key);
FILE* create_data_output_file(const char* filename);
    
// A simple structure to hold key-value pairs
typedef struct {
    char key[256];
    char value[256];
} config_entry;

// Assuming a small-medium number of configurations
config_entry config[256];
int config_size = 0;

// Structure to hold start and end times for a benchmark timer
struct timer {
    struct timespec start;
    struct timespec end;
};

static inline __attribute__((always_inline)) void timer_start(struct timer *timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->start);
}

static inline __attribute__((always_inline)) void timer_stop(struct timer *timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->end);
}

// Calculate elapsed time in nanoseconds
uint64_t get_elapsed_ns(struct timer *timer) {
    uint64_t start_ns = (uint64_t)timer->start.tv_sec * 1000000000L + timer->start.tv_nsec;
    uint64_t end_ns = (uint64_t)timer->end.tv_sec * 1000000000L + timer->end.tv_nsec;
    return end_ns - start_ns;
}

void setup() {
    // Any experimental prep work or setup goes here
}

void cleanup() {
    // Cleanup work goes here
}

static inline __attribute__((always_inline)) void benchmark_function() {
    // Benchmark workload
}

void benchmark() {
    struct timer *runs = mmap(NULL, sizeof(struct timer) * EXPERIMENT_LOOP_COUNT, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    memset(runs, 0, sizeof(struct timer) * EXPERIMENT_LOOP_COUNT);

    struct timespec prefault_ts;
    clock_gettime(CLOCK_MONOTONIC, &prefault_ts); // Prefault clock_gettime memory
    (void)prefault_ts;
    
    // Warmup phase
    for (int i = 0; i < (int)(EXPERIMENT_LOOP_COUNT * 0.01); ++i) {
        benchmark_function();
    }
    
    // Actual benchmark phase
    struct timer outer_timer;
    timer_start(&outer_timer);
    
    for (int i = 0; i < EXPERIMENT_LOOP_COUNT; ++i) {
#ifdef CONFIG_MEASURE_LATENCIES
        timer_start(&runs[i]);
#endif
        benchmark_function();

#ifdef CONFIG_MEASURE_LATENCIES
        timer_stop(&runs[i]);
#endif
    }

    timer_stop(&outer_timer);

#ifdef CONFIG_MEASURE_THROUGHPUT
    uint64_t elapsed_time = get_elapsed_ns(&outer_timer);
    
    printf("Total elapsed time  : %ld\n", elapsed_time);
    printf("Total iterations    : %d\n", EXPERIMENT_LOOP_COUNT);
    printf("Throughput          : %f iterations per second\n", EXPERIMENT_LOOP_COUNT / (elapsed_time / 1e9));
#endif

#ifdef CONFIG_MEASURE_LATENCIES
    FILE* log = create_data_output_file("latencies.csv");
    fprintf(log, "iteration,latency\n");
    for (int i = 0; i < EXPERIMENT_LOOP_COUNT; ++i) {
        uint64_t latency_measure = get_elapsed_ns(&runs[i]);
        fprintf(log, "%i,%ld\n", i, latency_measure);
    }
    fclose(log);
#endif
    
    munmap(runs, sizeof(struct timer) * EXPERIMENT_LOOP_COUNT);
}

int main() {
    load_config(CONFIG_PATH);
    
    EXPERIMENT_VERSION = get_config_string("experiment_version");
    EXPERIMENT_LOOP_COUNT = get_config_int("experiment_loop_count");
    EXPERIMENT_ITERATIONS = get_config_int("experiment_iterations");
    EXPERIMENT_RUN_ID = get_config_int("experiment_run_id");
    EXPERIMENT_CONFIGURATION_NAME = get_config_string("experiment_run_configuration");
    
    setup();
    benchmark();
    cleanup();
    
    free(EXPERIMENT_VERSION);
    free(EXPERIMENT_CONFIGURATION_NAME);
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation="
FILE* create_data_output_file(const char* filename) {
    char exe_path[PATH_MAX];
    ssize_t length = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (length == -1) {
        perror("readlink");
        return NULL;
    }
    exe_path[length] = '\0';

    // Construct the initial part of the path to the data directory
    char* dir = dirname(exe_path); // Get directory of the executable
    char data_dir_path[PATH_MAX];
    snprintf(data_dir_path, sizeof(data_dir_path), "%s/../data/raw/run_%d/%s",
             dir, EXPERIMENT_RUN_ID,
             EXPERIMENT_CONFIGURATION_NAME ? EXPERIMENT_CONFIGURATION_NAME : "default");

    // Tokenize the path and create directories one by one
    char* p = data_dir_path;
    for (p += 1; *p; p++) { // Skip the leading character assuming it's part of the path
        if (*p == '/') {
            *p = '\0'; // Temporarily end the string here to isolate the current directory component
            if (mkdir(data_dir_path, 0777) && errno != EEXIST) {
                fprintf(stderr, "Failed to create directory '%s': %s\n", data_dir_path, strerror(errno));
                return NULL;
            }
            *p = '/'; // Restore the slash to continue with the next directory component
        }
    }

    // Ensure the final directory is created
    if (mkdir(data_dir_path, 0777) && errno != EEXIST) {
        fprintf(stderr, "Failed to create directory '%s': %s\n", data_dir_path, strerror(errno));
        return NULL;
    }

    // Construct the full file path and attempt to open the file for writing
    char file_path[PATH_MAX];
    snprintf(file_path, PATH_MAX, "%s/%s", data_dir_path, filename);
    FILE* file = fopen(file_path, "w");
    if (!file) {
        perror("fopen");
        return NULL;
    }

    return file;
}
#pragma GCC diagnostic pop


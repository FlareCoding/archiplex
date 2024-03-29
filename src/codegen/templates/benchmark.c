#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>

#define CONFIG_PATH "../config/config.ini"

// Global configuration variables
char* EXPERIMENT_VERSION = NULL;
int EXPERIMENT_LOOP_COUNT = 0;
int EXPERIMENT_ITERATIONS = 0;
int EXPERIMENT_RUN_ID = 0;

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
    FILE* log = create_data_output_file("latencies");
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
    
    setup();
    benchmark();
    cleanup();
    
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

FILE* create_data_output_file(const char* filename) {
    char exe_path[PATH_MAX];
    ssize_t length = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (length == -1) {
        perror("readlink");
        return NULL;
    }
    exe_path[length] = '\0'; // Ensure null-termination

    // Find the last occurrence of '/' and terminate the string there
    char* last_slash = strrchr(exe_path, '/');
    if (last_slash == NULL) {
        fprintf(stderr, "Error finding executable directory\n");
        return NULL;
    }
    *last_slash = '\0'; // Cut off the executable name

    char run_dir_path[PATH_MAX];
    int needed = snprintf(run_dir_path, sizeof(run_dir_path), "%s/../data/raw/run_%d", exe_path, EXPERIMENT_RUN_ID);
    if (needed >= (int)sizeof(run_dir_path)) {
        fprintf(stderr, "Path is too long, truncation occurred\n");
        return NULL;
    }

    // Attempt to create the directory if it doesn't exist
    if (mkdir(run_dir_path, 0777) && errno != EEXIST) {
        perror("mkdir");
        return NULL;
    }

    char file_path[PATH_MAX];
    needed = snprintf(file_path, sizeof(file_path), "%s/%s", run_dir_path, filename);
    if (needed >= (int)sizeof(file_path)) {
        fprintf(stderr, "File path is too long, truncation occurred\n");
        return NULL;
    }

    // Attempt to open the file for writing (or creating if it doesn't exist)
    FILE* file = fopen(file_path, "w"); // Open for writing
    if (file == NULL) {
        perror("fopen");
    }

    return file;
}


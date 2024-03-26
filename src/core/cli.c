#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "libs/optparse.h"
#include "cli.h"
#include <limits.h>
#include <libgen.h>

void handle_help();
void handle_tools_help();
void handle_exp_help();
void handle_exp_list();
void handle_exp_create();
void handle_exp_delete(char *name);
void handle_exp_info(char *name);
void handle_sysinfo();

void launch_tool(const char *tool_name, char *const argv[]);

void cli_main(int argc, char **argv) {
    struct optparse options;
    optparse_init(&options, argv);
    char *arg = optparse_arg(&options); // Skip program name

    if (arg == NULL || strcmp(arg, "help") == 0) {
        handle_help();
    } else if (strcmp(arg, "tools") == 0) {
        arg = optparse_arg(&options);
        if (arg == NULL || strcmp(arg, "help") == 0) {
            handle_tools_help();
        } else {
            printf(COLOR_RED "Unknown tools command.\n" COLOR_RESET);
        }
    } else if (strcmp(arg, "exp") == 0) {
        arg = optparse_arg(&options);
        if (arg == NULL) {
            handle_exp_help();
        } else if (strcmp(arg, "help") == 0) {
            handle_exp_help();
        } else if (strcmp(arg, "list") == 0) {
            handle_exp_list();
        } else if (strcmp(arg, "create") == 0) {
            handle_exp_create();
        } else if (strcmp(arg, "delete") == 0) {
            char *name = optparse_arg(&options);
            if (name == NULL) {
                printf(COLOR_RED "Experiment name required for delete.\n" COLOR_RESET);
            } else {
                handle_exp_delete(name);
            }
        } else if (strcmp(arg, "info") == 0) {
            char *name = optparse_arg(&options);
            if (name == NULL) {
                printf(COLOR_RED "Experiment name required for info.\n" COLOR_RESET);
            } else {
                handle_exp_info(name);
            }
        } else {
            printf(COLOR_RED "Unknown exp command.\n" COLOR_RESET);
        }
    } else if (strcmp(arg, "sysinfo") == 0) {
        handle_sysinfo();
    } else {
        printf(COLOR_RED "Unknown command. Use 'archiplex help' for usage.\n" COLOR_RESET);
    }
}

void handle_help() {
    printf("\n");
    LOG_INFO("Archiplex CLI Help:\n\n");
    printf("  Usage: ");
    printf("archiplex [command] [options]\n\n");

    printf("  General Commands:\n");
    LOG_INFO("    help              ");
    printf("Displays this help message.\n");
    LOG_INFO("    sysinfo           ");
    printf("Lists system hardware and software info.\n\n");

    printf("  Experiment Commands:\n");
    LOG_INFO("    exp help          ");
    printf("Help for experiment-related commands.\n");
    LOG_INFO("    exp list          ");
    printf("List all saved experiments.\n");
    LOG_INFO("    exp create        ");
    printf("Launch an experiment creation tool.\n");
    LOG_INFO("    exp delete <name> ");
    printf("Delete an experiment by name.\n");
    LOG_INFO("    exp info <name>   ");
    printf("Display information about an experiment.\n\n");

    printf("  Tool Commands:\n");
    LOG_INFO("    tools help        ");
    printf("Display help for tool-related commands.\n\n");
}

void handle_tools_help() {
    printf(COLOR_CYAN "Tools Help:\n" COLOR_RESET);
    // List tools commands here
}

void handle_exp_help() {
    printf(COLOR_CYAN "Experimental Help:\n" COLOR_RESET);
}

void handle_exp_list() {
    printf(COLOR_CYAN "Experiments list:\n" COLOR_RESET);
}

void handle_exp_create() {
    printf(COLOR_CYAN "Prompt to create experiment:\n" COLOR_RESET);
}

void handle_exp_delete(char *name) {
    printf(COLOR_CYAN "Delete experiment '%s'\n" COLOR_RESET, name);
}

void handle_exp_info(char *name) {
    printf(COLOR_CYAN "Get experiment info for '%s'\n" COLOR_RESET, name);
}

void handle_sysinfo() {
    char *argv[] = { "sysinfo.sh", NULL };
    launch_tool("sysinfo.sh", argv);
}

void launch_tool(const char *tool_name, char *const argv[]) {
    char exec_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exec_path, sizeof(exec_path) - 1);
    if (len != -1) {
        exec_path[len] = '\0'; // Ensure null-terminated string
        char *dir = dirname(exec_path); // Get directory of the current executable
        char *parent_dir = dirname(dir); // Assumes executable is in archiplex/bin/
        
        char tool_path[PATH_MAX];
        snprintf(tool_path, sizeof(tool_path), "%s/tools/%s", parent_dir, tool_name); // Construct tool path
        
        // Execute the tool
        execv(tool_path, argv);
        
        // If execv returns, there was an error
        perror("Failed to execute tool");
    } else {
        perror("Failed to resolve executable path");
    }
}


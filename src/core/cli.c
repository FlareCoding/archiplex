#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "libs/optparse.h"
#pragma GCC diagnostic pop

#include "cli.h"
#include <limits.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define INSTALL_DIRECTORY "/usr/local"

void handle_help();
void handle_tools_help();
void handle_exp_help();
void handle_exp_list();
void handle_exp_create();
void handle_exp_delete(char *name);
void handle_exp_info(char *name);
void handle_sysinfo();

void get_archiplex_root_dir(char *root_path);
void get_archiplex_experiments_dir(char *dir);
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
        } else if (strcmp(arg, "run") == 0) {
            char *path_to_experiment_dir = NULL;
            char *config_name = "";
            int verbose = 0; // Verbose flag

            // Process further arguments to find optional parameters
            while ((arg = optparse_arg(&options)) != NULL) {
                if (strcmp(arg, "-c") == 0) { // Next argument is the config name
                    config_name = optparse_arg(&options);
                    if (config_name == NULL) {
                        printf(COLOR_RED "Expected configuration name after '-c'.\n" COLOR_RESET);
                        return;
                    }
                } else if (strcmp(arg, "-v") == 0) {
                    verbose = 1;
                } else if (strcmp(arg, "-vv") == 0) {
                    verbose = 2;
                } else {
                    // Treat as the path if not a recognized option
                    if (path_to_experiment_dir == NULL) {
                        path_to_experiment_dir = arg;
                    } else {
                        // More than one path-like argument encountered
                        printf(COLOR_RED "Unexpected argument: %s\n" COLOR_RESET, arg);
                        return;
                    }
                }
            }

            // Default to current working directory if no path is provided
            char cwd[PATH_MAX];
            if (path_to_experiment_dir == NULL) {
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    path_to_experiment_dir = cwd;
                } else {
                    perror("getcwd");
                    return;
                }
            }

            char *tool_args[] = {
                "run_experiment.sh",
                path_to_experiment_dir,
                config_name,
                (verbose == 2) ? "-vv" : (verbose ? "-v" : NULL),
                NULL
            };
            launch_tool("run_experiment.sh", tool_args);
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
    LOG_INFO("    exp run [path] [-c <config>,<config2>,...] [-v] [-vv]\n");
    printf("                      Runs the experiment in the current directory unless specifies otherwise.\n");
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
    printf("\n");
    printf("  Usage: ");
    printf("archiplex exp [options]\n\n");

    printf("  General Commands:\n");
    LOG_INFO("    help                                         ");
    printf("Displays this help message.\n");

    LOG_INFO("    list                                         ");
    printf("Displays a list of registered experiments.\n");

    LOG_INFO("    create                                       ");
    printf("Launches an experiment creation wizard.\n");

    LOG_INFO("    delete <name>                                ");
    printf("Deletes the specified experiment.\n");

    LOG_INFO("    run [path] [-c <config>,<config2>,...] [-v]  ");
    printf("Runs the experiment in the current directory unless specifies otherwise. Optionally can specify the configuratioin to run\n");

    LOG_INFO("    info   <name>                                ");
    printf("Displays information related to a specified experiment.\n\n");
}

void handle_exp_list() {
    char experiments_path[PATH_MAX];
    get_archiplex_experiments_dir(experiments_path);

    DIR *d = opendir(experiments_path);
    if (d) {
        struct dirent *dir;
        LOG_INFO("Registered experiments:\n");
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
                printf("  - %s\n", dir->d_name);
            }
        }
        closedir(d);
    } else {
        perror("Failed to open experiments directory");
    }
}

void handle_exp_create() {
    char root_dir[PATH_MAX];
    get_archiplex_root_dir(root_dir);

    char codegen_script_path[PATH_MAX];
    int needed = 0;
    if (strncmp(root_dir, INSTALL_DIRECTORY, strlen(INSTALL_DIRECTORY)) == 0) {
        needed = snprintf(codegen_script_path, sizeof(codegen_script_path), "%s/codegen/codegen.py", root_dir);
    } else {
        needed = snprintf(codegen_script_path, sizeof(codegen_script_path), "%ssrc/codegen/codegen.py", root_dir);
    }

    if (needed >= sizeof(codegen_script_path)) {
        fprintf(stderr, "Error: Path too long.\n");
        return; // Exit the function if path is too long
    }

    // Fork a process to run the Python script
    pid_t pid = fork();
    if (pid == -1) {
        // Fork failed
        perror("fork");
        return;
    } else if (pid == 0) {
        // Child process: Execute the codegen Python script
        // Assuming Python executable is in the user's PATH
        execlp("python3", "python3", codegen_script_path, NULL);

        // If execlp returns, an error occurred
        perror("execlp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process: Wait for the child to complete
        int status;
        waitpid(pid, &status, 0);
    }
}

void handle_exp_delete(char *name) {
    char experiments_dir[PATH_MAX];
    get_archiplex_experiments_dir(experiments_dir);

    char experiment_path[PATH_MAX];
    int needed = snprintf(experiment_path, sizeof(experiment_path), "%s/%s", experiments_dir, name);
    
    if (needed >= sizeof(experiment_path)) {
        fprintf(stderr, "Error: Path too long.\n");
        return; // Exit the function if path is too long
    }

    // Confirm with the user
    printf("Are you sure you want to delete the experiment '%s'? [y/N]: ", name);
    int response = getchar();
    if (response != 'y' && response != 'Y') {
        printf("Deletion cancelled.\n");
        return; // Do not proceed with deletion
    }
    
    // Clear input buffer
    while (getchar() != '\n');

    // Check if the directory exists
    struct stat statbuf;
    if (stat(experiment_path, &statbuf) == -1 || !S_ISDIR(statbuf.st_mode)) {
        perror("Experiment not found");
        return;
    }

    // Use fork and exec pattern to call 'rm -rf'
    pid_t pid = fork();
    if (pid == 0) { // Child process
        execl("/bin/rm", "rm", "-rf", experiment_path, NULL);
        // If execl returns, an error occurred
        perror("execl");
        exit(1);
    } else if (pid > 0) { // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to finish
    } else {
        // Fork failed
        perror("fork");
    }
}

void handle_exp_info(char *name) {
    printf(COLOR_CYAN "Get experiment info for '%s'\n" COLOR_RESET, name);
}

void handle_sysinfo() {
    char *argv[] = { "sysinfo.sh", NULL };
    launch_tool("sysinfo.sh", argv);
}

void get_archiplex_root_dir(char *root_path) {
    char exec_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exec_path, sizeof(exec_path) - 1);
    if (len != -1) {
        exec_path[len] = '\0'; // Ensure null-terminated string
        char *dir = dirname(exec_path); // Get directory of the current executable
        char *parent_dir = dirname(strdup(dir)); // Duplicate since dirname can modify the input

        if (strncmp(parent_dir, INSTALL_DIRECTORY, strlen(INSTALL_DIRECTORY)) == 0) {
            snprintf(root_path, PATH_MAX, "%s/%s/", INSTALL_DIRECTORY, "share/archiplex");
        } else {
            snprintf(root_path, PATH_MAX, "%s/", parent_dir); // Construct the root path
        }
    } else {
        perror("Failed to resolve executable path");
        exit(1); // Exiting as we can't proceed without the path
    }
}

void get_archiplex_experiments_dir(char *dir) {
    char exec_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exec_path, sizeof(exec_path) - 1);
    if (len == -1) {
        perror("Failed to resolve executable path");
        exit(1); // Exiting as we can't proceed without the path
    }
        
    exec_path[len] = '\0'; // Ensure null-terminated string
    char *exec_dir = dirname(exec_path); // Get directory of the current executable
    char *parent_dir = dirname(strdup(exec_dir)); // Duplicate since dirname can modify the input

    if (strncmp(parent_dir, INSTALL_DIRECTORY, strlen(INSTALL_DIRECTORY)) == 0) {
        // Get the HOME environment variable to find the user's home directory
        const char *home = getenv("HOME");
        if (home != NULL) {
            // Ensure that the path to "/experiments" will not overflow the buffer
            // Assuming `dir` has enough space allocated for this purpose
            snprintf(dir, PATH_MAX, "%s/experiments", home);
        } else {
            // Fallback if HOME is not set (unlikely in a normal user environment)
            fprintf(stderr, "Error: HOME environment variable is not set.\n");
            exit(1);
        }
    } else {
        snprintf(dir, PATH_MAX, "%s/experiments", parent_dir);
    }
}

void launch_tool(const char *tool_name, char *const argv[]) {
    char root_dir[PATH_MAX];
    get_archiplex_root_dir(root_dir);

    char tool_path[PATH_MAX] = { 0 };
    int res = snprintf(tool_path, sizeof(tool_path), "%stools/%s", root_dir, tool_name); // Construct tool path
    if (res >= sizeof(tool_path)) {
        fprintf(stderr, "Error: Path too long.\n");
        return;
    }
    
    // Execute the tool
    execv(tool_path, argv);
    
    // If execv returns, there was an error
    perror("Failed to execute tool");
}


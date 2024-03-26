#ifndef CLI_H
#define CLI_H
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// ANSI color codes
#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_WHITE   "\x1b[37m"

#define LOG_ERROR(fmt, ...) printf(COLOR_RED fmt COLOR_RESET, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  printf(COLOR_YELLOW fmt COLOR_RESET, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  printf(COLOR_CYAN fmt COLOR_RESET, ##__VA_ARGS__)
#define LOG_SUCCESS(fmt, ...) printf(COLOR_GREEN fmt COLOR_RESET, ##__VA_ARGS__)

void cli_main(int argc, char **argv);

#endif // CLI_H

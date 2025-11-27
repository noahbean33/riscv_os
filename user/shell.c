#include "include/stdio.h"
#include "include/malloc.h"
#include "include/string.h"
#include "include/syscall.h"
#include "include/log.h"

#define MAXLINE 128
#define MAX_ARGS 8

void print_help(void) {
    puts("Available commands:\n");
    puts("  help         Show this overview\n");
    puts("  hello        Show the hello message\n");
    puts("  ps           Show list of active processes\n");
    puts("  ls           Show files in the current directory\n");
    puts("  mem          Show memory usage / available memory\n");
    puts("  date         Show the current date and time\n");
    puts("  echo [args]  Show the specified text\n");
    puts("  clear        Clear the screen\n");
}

int main() {
    char line[MAXLINE];
    char filename[MAXLINE + 8];  // room for ".elf"
    char *title = "Novix RISC-V 64 OS, (c) NovixManiac, Shell version : 0.0.1\n";

    LOG("started...");

    cls();
    puts(title);

    while (1) {
        // 1. Show prompt
        puts("$ ");

        int pos = 0;
        char c;

        // 2. Read character by character and echo directly
        while (1) {
            int n = read(0, &c, 1);
            if (n != 1) continue;
            putc(c);

            // Pressed ENTER? Done with input
            if (c == '\r' || c == '\n') {
                putc('\n');
                break;
            }

            // BACKSPACE (ASCII 0x7F of '\b')
            if ((c == 0x7F || c == '\b') && pos > 0) {
                pos--;
                // move cursor back, erase character, and cursor back
                puts("\b \b");
                continue;
            }

            if (pos < MAXLINE - 1) {
                line[pos++] = c;
            }
        }

        line[pos] = '\0'; // null-terminate string

        // 3. Skip empty input
        if (line[0] == '\0') continue;

         // 4a. Program arguments
        char* argv[MAX_ARGS];
        int argc = 0;

        // 4b. Tokenize input
        char* token = strtok(line, " ");
        while (token && argc < MAX_ARGS) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }

        if (argc == 0) continue;  // emty row

        char* cmd = argv[0];

        // 5a Built-in "clear"
        if (!strcmp(line, "clear")) {
            cls(); 
            puts(title); 
            continue;
        }

        // 5b Built-in "help"
        if (!strcmp(line, "help")) {
            print_help();
            continue;
        }

        // 5c Not allowed to start
        if (!strcmp(line, "shell")) {
            printf("[shell] Command '%s' Not allowed to execute\n", line);
            continue;
        }

        if (!strcmp(line, "idle")) {
            printf("[shell] Command '%s' Not allowed to execute\n", line);
            continue;
        }

        if (!strcmp(line, "init")) {
            printf("[shell] Command '%s' Not allowed to execute\n", line);
            continue;
        }

        // 6. Add ".elf"
        strcpy(filename, cmd);  // cmd == argv[0]
        strcat(filename, ".elf");

        if (!tarfs_exists(filename)) {
            printf("[shell] Command '%s' not found\n", line);
            continue;
        }

        // 7. Fork / exec / wait / exit
        int pid = fork();
        if (pid == 0) {
            exec(filename, argv, argc);
            puts("[shell] exec failed\n");
            exit(1);
        } else if (pid > 0) {
            int status;
            wait(&status);
        } else {
            puts("[shell] fork failed\n");
        }
    }

    return 0;
}

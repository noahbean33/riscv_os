#pragma once

#include <stdint.h>

#define MAX_ARGS 8
#define MAX_ARG_LENGTH 128


char** setup_user_arguments(char argv_kernel[MAX_ARGS][MAX_ARG_LENGTH], 
       int argc, uint64_t *sp_ptr);

void debug_argv(int argc, char** argv);
void debug_argv_userstack(int argc, char** argv_user);
#include "arguments.h"
#include "kernel.h"
#include "heap.h"
#include "string.h"
#include "process.h"
#include "uart.h"
#include "syscall.h"
#include "debug.h"

char** setup_user_arguments(char argv_kernel[MAX_ARGS][MAX_ARG_LENGTH], int argc, uint64_t *sp_ptr) {
    uint64_t sp = *sp_ptr;
    char* argv_ptrs[MAX_ARGS + 1];  // temporarily in kernel

    // Copy argument strings
    for (int i = argc - 1; i >= 0; i--) {
        size_t len = strnlen(argv_kernel[i], MAX_ARG_LENGTH) + 1;
        sp -= len;
        sp &= ~0x7UL;

        if (sp < g_user_stack_bottom)
            PANIC("[setup_user_arguments] stack overflow while copying argv");

        enable_sum();
        memcpy((void*)sp, argv_kernel[i], len);
        disable_sum();

        argv_ptrs[i] = (char*)sp;  // user stack address!
    }

    argv_ptrs[argc] = NULL;

    // Copy argv[] pointers to stack yourself
    sp -= (argc + 1) * sizeof(char*);
    sp &= ~0x7UL;

    if (sp < g_user_stack_bottom)
        PANIC("[setup_user_arguments] stack overflow during argv[] copy");

    enable_sum();
    memcpy((void*)sp, argv_ptrs, (argc + 1) * sizeof(char*));
    disable_sum();

    *sp_ptr = sp;
    return (char**)sp;
}

void debug_argv(int argc, char** argv) {
    LOG_USER_DBG("🔍 debug_argv(argc=%d)", argc);
    for (int i = 0; i < argc; i++) {
        LOG_USER_DBG("  argv[%d] = %s", i, argv[i]);
    }
}

void debug_argv_userstack(int argc, char** argv_user) {
    uart_printf("🧠 debug_argv_userstack(argc=%d)\n", argc);

    enable_sum();  // required for secure access to user space memory

    for (int i = 0; i < argc; i++) {
        char* user_str = argv_user[i];
        if ((uint64_t)user_str < g_user_stack_bottom || (uint64_t)user_str >= g_user_stack_top) {
            uart_printf("  argv[%d] = INVALID POINTER (0x%lx)\n", i, (uint64_t)user_str);
        } else {
            uart_printf("  argv[%d] = \"%s\"\n", i, user_str);
        }
    }

    uart_printf("  argv[%d] = %p (should be NULL)\n", argc, argv_user[argc]);

    disable_sum();
}
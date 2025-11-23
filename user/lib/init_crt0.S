# used for only the first init process like shell.elf (only) or init.elf
# all order user progs build with crt0.o

.section .text.start
.global start

start:
    la sp, __user_stack_top
    call main
    call exit
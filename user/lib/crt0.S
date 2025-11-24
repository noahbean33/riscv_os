# used for all user programs other then the first (init) process or shell (only) process

.section .text.start
.global start

start:
    
    call main
    call exit
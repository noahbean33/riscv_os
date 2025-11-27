#pragma once

#include <stdint.h>

#define SBI_SET_TIMER   0x00
#define SBI_EXT_TIMER   0x54494D45

struct sbiret
{
    long error;
    long value;
};

/* sbi_call: wrapper to issue ECALL to machine mode.
 * Arguments: arg0..arg5 in a0..a5, fid in a6, eid in a7
 * Returns sbiret with error (a0) and value (a1)
 */
struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4,
                       long arg5, long fid, long eid);

void sbi_set_timer(uint64_t stime_value);

#include "include/stdio.h"
#include "include/syscall.h"
#include "include/time.h"

int main(void) {
    struct DateTime now;
    if (syscall(SYS_GET_TIME, (uint64_t)&now, 0, 0) < 0) {
        printf("date: syscall failed\n");
        return -1;
    }

    char ampm[] = "AM";
    int hour = now.hour;
    if (hour >= 12) {
        ampm[0] = 'P';
        if (hour > 12) hour -= 12;
    } else if (hour == 0) {
        hour = 12;
    }

    printf("%s %s %02d %02d:%02d:%02d %s CEST %d\n",
        WEEKDAYS[now.weekday],
        MONTHS[now.month - 1],
        now.day,
        hour, now.minute, now.second,
        ampm,
        now.year);

    return 0;
}
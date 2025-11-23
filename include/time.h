#pragma once

#include <stdint.h>

struct DateTime {
    int year, month, day;
    int hour, minute, second;
    int weekday;  // 0 = Sunday ... 6 = Saturday
};

static const char *weekday_names[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
static const int month_days[] = {
    31,28,31,30,31,30,31,31,30,31,30,31
};

void compute_datetime_from_epoch(uint64_t epoch, struct DateTime *dt);
void print_datetime(struct DateTime *dt);
void compute_datetime(uint64_t ticks, struct DateTime *dt);
uint64_t read_clint_mtime(void);
uint64_t get_time();
#include "time.h"
#include "uart.h"
#include "build_time.h"  // Contains: BUILD_YEAR, BUILD_MONTH, etc.

#define TIMEZONE_OFFSET_SECS (2 * 3600)     // Example: CEST = UTC+2
#define TICKS_PER_SECOND 10000000UL

uint64_t compute_epoch(int year, int month, int day, int hour, int minute, int second) {
    uint64_t days = 0;
    for (int y = 1970; y < year; y++) {
        days += (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) ? 366 : 365;
    }
    for (int m = 0; m < month - 1; m++) {
        days += month_days[m];
        if (m == 1 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
            days++;
        }

    }
    days += day - 1;
    return days * 86400 + hour * 3600 + minute * 60 + second;
}

void compute_datetime_from_epoch(uint64_t epoch, struct DateTime *dt) {
    epoch += TIMEZONE_OFFSET_SECS;
    uint64_t days = epoch / 86400;
    uint64_t secs = epoch % 86400;
    dt->hour = (secs / 3600) % 24;
    dt->minute = (secs / 60) % 60;
    dt->second = secs % 60;

    int year = 1970;
    while (1) {
        int leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        int days_in_year = leap ? 366 : 365;
        if (days < (uint64_t)days_in_year) break;
        days -= days_in_year;
        year++;
    }
    dt->year = year;

    int month = 0;
    while (1) {
        int dim = month_days[month];
        if (month == 1 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) dim++;
        if (days < (uint64_t)dim) break;
        days -= dim;
        month++;
    }
    dt->month = month + 1;
    dt->day = days + 1;

    // 1970-01-01 was Thursday (4)
    dt->weekday = (epoch / 86400 + 4) % 7;
}

void compute_datetime(uint64_t ticks, struct DateTime *dt) {
    uint64_t seconds_since_boot = ticks / TICKS_PER_SECOND;
    uint64_t base_epoch = compute_epoch(BUILD_YEAR, BUILD_MONTH, BUILD_DAY,
                                        BUILD_HOUR, BUILD_MINUTE, BUILD_SECOND);
    uint64_t current_epoch = base_epoch + seconds_since_boot;
    compute_datetime_from_epoch(current_epoch, dt);
}

void print_datetime(struct DateTime *dt) {
    char ampm[] = "AM";
    int hour = dt->hour;
    if (hour >= 12) { ampm[0] = 'P'; if (hour > 12) hour -= 12; }
    else if (hour == 0) { hour = 12; }

    uart_printf("%s %s %d %d:%d:%d %s %d\n",
        weekday_names[dt->weekday],
        (const char *[]){ "Jan","Feb","Mar","Apr","May","Jun",
                          "Jul","Aug","Sep","Oct","Nov","Dec" }[dt->month - 1],
        dt->day, hour, dt->minute, dt->second, ampm, dt->year);
}

uint64_t get_time() {
    uint64_t value;
    __asm__ volatile("rdtime %0" : "=r"(value));
    return value;
}
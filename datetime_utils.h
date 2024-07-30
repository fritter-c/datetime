#ifndef DATETIME_FUNCS_H
#define DATETIME_FUNCS_H
#include "datetime.h"

inline long long microseconds_in_between(gtr::datetime dt1, gtr::datetime dt2) {
    return dt2.data - dt1.data;
}

inline long long seconds_in_between(gtr::datetime dt1, gtr::datetime dt2) {
    return microseconds_in_between(dt1, dt2) / 1000000LL;
}

inline long long minutes_in_between(gtr::datetime dt1, gtr::datetime dt2) {
    return seconds_in_between(dt1, dt2) / 60LL;
}

inline long long hours_in_between(gtr::datetime dt1, gtr::datetime dt2) {
    return seconds_in_between(dt1, dt2) / 60LL;
}

inline long long days_in_between(gtr::datetime dt1, gtr::datetime dt2) {
    return hours_in_between(dt1, dt2) / 24LL;
}

inline long long months_in_between(gtr::datetime dt1, gtr::datetime dt2) {
    gtr::datetime_pack p1, p2;
    dt1.to_pack(p1);
    dt2.to_pack(p2);
    int month_diff;
    if (p1.year > p2.year)
        month_diff = p1.month - p2.month;
    else
        month_diff = p2.month - p1.month;

    return (p2.year - p1.year) * 12 + month_diff;
}

inline long long years_in_between(gtr::datetime dt1, gtr::datetime dt2) {
    return months_in_between(dt1, dt2) / 12;
}

#endif

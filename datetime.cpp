#include "datetime.h"
#ifdef HAS_STD_CHRONO
#include <chrono>
#endif
#include "datetime_parser.h"
namespace gtr {
constexpr inline bool is_leap_year(int year) { return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0; }
constexpr unsigned int monthdays[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365}; // Non-leap year
constexpr int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};                    // Non-leap year

static inline constexpr unsigned int days_until_month(const int year, const int month) {
    return (is_leap_year(year) ? monthdays[month - 1] + (month > 1) : monthdays[month - 1]);
}

static inline constexpr unsigned int leap_years_count(int start_year, int end_year) {
    start_year--;
    return ((end_year / 4) - (end_year / 100) + (end_year / 400)) - ((start_year / 4) - (start_year / 100) + (start_year / 400));
}

static inline constexpr void epoch_to_datetime_pack(long long time, datetime_struct &pack) {
    // Set microseconds
    if (time > 0)
        pack.microsecond = time % 1000000LL;
    else
        pack.microsecond = -time % 1000000LL;

    time /= 1000000LL;

    // Adapted from sourceware NewLib
    long long days = time / (24 * 60 * 60) + 719468L;
    long long sec_per_day = time % (24 * 60 * 60);
    if (sec_per_day < 0) {
        sec_per_day += 24 * 60 * 60;
        days--;
    }

    int era = (days >= 0 ? days : days - 146097LL + 1) / 146097LL;
    unsigned long long era_day = days - era * 146097LL;
    unsigned int era_year = (era_day - era_day / (1461 - 1) + era_day / 36524LL - era_day / (146097LL - 1)) / 365;
    unsigned int year_day = era_day - (365 * era_year + era_year / 4 - era_year / 100);
    unsigned month = (5 * year_day + 2) / 153;
    unsigned day = year_day - (153 * month + 2) / 5 + 1;
    month += month < 10 ? 2 : -10;
    int year = era_year + era * 400 + (month <= 1);
    // Set the year
    pack.year = year;

    // Set the month and month day
    pack.month = month + 1;
    pack.day = day;

    // Set hour, minute and second
    pack.hour = sec_per_day / (60 * 60);
    pack.minute = (sec_per_day % (60 * 60)) / 60;
    pack.second = (sec_per_day % (60 * 60)) % 60;
}

static inline constexpr long long seconds_since_epoch(const int day, const int month, const int year, const int hour, const int minute,
                                                      const int second) {

    // Reference https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04
    int day_corrected = days_until_month(year, month > 0 && month <= 12 ? month : 12) + day - 1;
    if (month == 2 && is_leap_year(year)) {
        day_corrected--;
    }
    if (year >= 1970) {
        long long year_corrected = year - 1900;
        return second + minute * 60 + hour * 3600 + day_corrected * 86400LL + (year_corrected - 70) * 31536000 +
               ((year_corrected - 69) / 4) * 86400 - ((year_corrected - 1) / 100) * 86400 + ((year_corrected + 299) / 400) * 86400;
    }
    unsigned int leaps = leap_years_count(year, 1970);
    if (year < 0 && is_leap_year(year))
        leaps++;
    int normal = (1970 - year) - leaps;
    int total_days = leaps * 366LL + normal * 365LL;
    total_days -= day_corrected;
    return (second + minute * 60 + hour * 3600 - total_days * 86400LL);
}

/**
 * Converts a datetime to a string based on the specified format.
 *
 * Supported format options:
 * - YYYY: Four-digit year (e.g., 2024)
 * - YF: All digits(e.g, -10,000)
 * - YY: Two-digit year (e.g., 24)
 * - MM: Two-digit month (01 to 12)
 * - MMM: Three-letter abbreviation of the month (e.g., Jan, Feb)
 * - DD: Two-digit day (01 to 31)
 * - hh: Two-digit hour (00 to 23)
 * - mm: Two-digit minute (00 to 59)
 * - ss: Two-digit second (00 to 59)
 * - z to zzzzzz: One to six-digit microsecond (0 to 999999)
 * - Any other characters in the format string are copied directly to the output.
 *
 * Example format strings:
 * - "YYYY-MM-DD hh:mm:ss" -> "2024-07-22 14:35:45"
 * - "DD/MM/YYYY" -> "22/07/2024"
 * - "MA DD, YYYY" -> "Ju 22, 2024"
 * - "hh:mm:ss.uuuuuu" -> "14:35:45.123456"
 *
 */
static bool datetime_to_string(datetime date, char *out, const char *format = DATETIME_DEFAULT_FORMAT,
                               date_format group_format = date_format::text_date) {
    if (group_format == date_format::text_date) {
        datetime_struct pack;
        date.to_pack(pack);
        const char *state = format;
        char *out_ptr = out;
        while (*state != '\0') {
            switch (*state) {
            case 'D':
                day_field::puts(&state, &out_ptr, pack);
                break;
            case 'M':
                if (*(state + 1) == 'M' && *(state + 2) == 'M')
                    month_field<month_format::month_abbrev>::puts(&state, &out_ptr, pack);
                else
                    month_field<>::puts(&state, &out_ptr, pack);
                break;
            case 'Y':
                year_field<>::puts(&state, &out_ptr, pack);
                break;
            case 'h':
                hour_field::puts(&state, &out_ptr, pack);
                break;
            case 'm':
                minute_field::puts(&state, &out_ptr, pack);
                break;
            case 's':
                second_field::puts(&state, &out_ptr, pack);
                break;
            case 'z':
                microsecond_field<>::puts(&state, &out_ptr, pack);
                break;
            default:
                separator_field<>::puts(&state, &out_ptr, pack);
                break;
            }
        }
        end_string(out_ptr);
        return true;
    }
    return datetime_to_string(date, out, "YYYY-MM-DDThh:mm:ss+00:00", date_format::text_date);
}

static long long parse_datetime_string(const char *date, const char *format, date_format group_format = date_format::text_date) {
    const char *state = format;
    const char *date_char = date;
    datetime_struct pack{};
    if (group_format == date_format::text_date) {
        while (*state != '\0') {
            switch (*state) {
            case 'M':
                if (*(state + 1) == 'M' && *(state + 2) == 'M')
                    state += month_field<month_format::month_abbrev>::parse(&date_char, pack);
                else
                    state += month_field<>::parse(&date_char, pack);
                break;
            case 'Y':
                state += year_field<>::parse(&date_char, pack);
                break;
            case 'D':
                state += day_field::parse(&date_char, pack);
                break;
            case 'h':
                state += hour_field::parse(&date_char, pack);
                break;
            case 'm':
                state += minute_field::parse(&date_char, pack);
                break;
            case 's':
                state += second_field::parse(&date_char, pack);
                break;
            case 'z':
                state += microsecond_field<>::parse(&date_char, pack);
                break;
            default:
                state += separator_field<>::parse(&date_char, pack);
                break;
            }
        }
        return datetime(pack.day, pack.month, pack.year, pack.hour, pack.minute, pack.second, pack.microsecond).data;
    }
    return parse_datetime_string(date, "YYYY-MM-DDThh:mm:ss+00:00", date_format::text_date);
}

long long datetime_struct::to_datetime() { return seconds_since_epoch(day, month, year, hour, minute, second) * 1000000LL + microsecond; }

datetime::datetime(int day, int month, int year, int hour, int minute, int second, int microsecond) {
    data = seconds_since_epoch(day, month, year, hour, minute, second) * 1000000LL + microsecond;
}

datetime::datetime(const char *date, const char *format, date_format group_format) { from_string(date, format, group_format); }

bool datetime::from_string(const char *date, const char *format, date_format group_format) {
    data = parse_datetime_string(date, format, group_format);
    return data != DATETIME_INVALID;
}

bool datetime::to_string_format(char *out, const char *format, date_format group_format) const {
    return datetime_to_string(*this, out, format, group_format);
}

void datetime::add_months(int months) {
    datetime_struct pack;
    to_pack(pack);
    const int new_month = pack.month + months;

    if (new_month > 12) {
        pack.year = pack.year + new_month / 12;
        pack.month = new_month % 12 == 0 ? 1 : new_month % 12;
    } else if (new_month < 1) {
        pack.year = pack.year + new_month / 12 - 1;
        pack.month = 12 + new_month % 12;
    } else {
        pack.month = new_month;
    }
    if (pack.day > days_in_month[pack.month - 1]) {
        pack.day = days_in_month[pack.month - 1];
    }
    *this = datetime(pack.day, pack.month, pack.year, pack.hour, pack.minute, pack.second, pack.microsecond);
}

void datetime::add_years(int years) {
    datetime_struct pack;
    to_pack(pack);
    pack.year += years;
    constexpr int february_in_leap = 29;
    if (pack.day > days_in_month[pack.month - 1]) {
        pack.day = days_in_month[pack.month - 1];
    }
    if (is_leap_year(pack.year) && pack.month == 2) {
        pack.day = february_in_leap; // February in a leap year
    }
    *this = datetime(pack.day, pack.month, pack.year, pack.hour, pack.minute, pack.second, pack.microsecond);
}

datetime datetime::begin_of_the_day() const { return this->data - (this->data % 86400000000LL); }

datetime datetime::end_of_the_day() const { return begin_of_the_day() + 86399999999LL; }

bool datetime::different_day(datetime other) const {
    constexpr long long seconds_per_day = 86400LL;
    // Convert microseconds to seconds.
    long long t1 = data / 1000000LL;
    long long t2 = other.data / 1000000LL;

    // Compute day index (floor division for negative times)
    long long day1 = t1 / seconds_per_day;
    long long day2 = t2 / seconds_per_day;
    if (t1 % seconds_per_day < 0)
        day1--;
    if (t2 % seconds_per_day < 0)
        day2--;

    return day1 != day2;
}

bool datetime::different_month(datetime other) const {
    datetime_struct pack1;
    epoch_to_datetime_pack(data, pack1);
    datetime_struct pack2;
    epoch_to_datetime_pack(other.data, pack2);
    // Two datetimes are in different months if either the year or the month differ.
    return (pack1.year != pack2.year) || (pack1.month != pack2.month);
}

bool datetime::different_year(datetime other) const {
    datetime_struct pack1;
    epoch_to_datetime_pack(data, pack1);
    datetime_struct pack2;
    epoch_to_datetime_pack(other.data, pack2);
    return pack1.year != pack2.year;
}

void datetime::to_pack(datetime_struct &pack) const { epoch_to_datetime_pack(data, pack); }

int datetime::day() const {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);
    return pack.day;
}

int datetime::month() const {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);
    return pack.month;
}

int datetime::year() const {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);
    return pack.year;
}

int datetime::second() const {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);
    return pack.second;
}

int datetime::minute() const {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);
    return pack.minute;
}

int datetime::hour() const {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);
    return pack.hour;
}

int datetime::microsecond() const {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);
    return pack.microsecond;
}

int datetime::get_minute_of_day() const {
    // Optimized
    long long total_seconds = data / 1000000LL;
    const long long seconds_per_day = 86400LL;
    long long seconds_since_midnight = (total_seconds % seconds_per_day + seconds_per_day) % seconds_per_day;
    int minutes_since_midnight = (int)(seconds_since_midnight / 60);
    return minutes_since_midnight;
}

int datetime::get_second_of_day() const {
    // Optimized
    long long total_seconds = data / 1000000LL;
    const long long seconds_per_day = 86400LL;
    int seconds_since_midnight = (int)((total_seconds % seconds_per_day + seconds_per_day) % seconds_per_day);
    return seconds_since_midnight;
}

int datetime::get_microsecond_of_day() const {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);
    return pack.hour * 3600 * 1000000 + pack.minute * 60 * 1000000 + pack.second * 1000000 + pack.microsecond;
}

int datetime::day_of_week() const {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);
    int month = pack.month;
    int year = pack.year;
    int day = pack.day;
    if (month < 3) {
        month += 12;
        year--;
    }
    int K = year % 100;
    int J = year / 100;
    int h = (day + 13 * (month + 1) / 5 + K + K / 4 + J / 4 + 5 * J) % 7;
    return (h + 6) % 7;
}

datetime datetime::date() const {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);
    return datetime(seconds_since_epoch(pack.day, pack.month, pack.year, 0, 0, 0) * 1000000LL);
}

datetime datetime::end_of_the_month() const {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);
    int day = days_in_month[pack.month - 1];
    if (pack.month == 2 && is_leap_year(pack.year))
        day = 29;
    return datetime(day, pack.month, pack.year, 23, 59, 59, 999999);
}

int datetime::month_day_count(int month, int year) {
    if (month == 2)
        return is_leap_year(year) ? 29 : 28;
    return days_in_month[month - 1];
}

bool datetime::leap_year() {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);
    return is_leap_year(pack.year);
}

unsigned int datetime::seconds_in_month() {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);
    int days = month_day_count(pack.month, pack.year);
    return days * 86400;
}

unsigned long long datetime::seconds_in_range(int start_year, int start_month, int end_year, int end_month) {
    const int seconds_per_day = 86400;
    unsigned long long total_days = 0;

    if (start_year == end_year) {
        int start_cumulative = monthdays[start_month - 1];
        int end_cumulative = monthdays[end_month];
        if (is_leap_year(start_year)) {
            if (start_month > 2)
                start_cumulative += 1;
            if (end_month > 2)
                end_cumulative += 1;
        }
        total_days = end_cumulative - start_cumulative;
    } else {
        int start_cumulative = monthdays[start_month - 1];
        if (is_leap_year(start_year) && start_month > 2)
            start_cumulative += 1;
        int days_in_start_year = is_leap_year(start_year) ? 366 : 365;
        total_days += days_in_start_year - start_cumulative;

        // For any full intermediate years, add all days.
        for (int year = start_year + 1; year < end_year; ++year) {
            total_days += is_leap_year(year) ? 366 : 365;
        }
        int end_cumulative = monthdays[end_month];
        if (is_leap_year(end_year) && end_month > 2)
            end_cumulative += 1;
        total_days += end_cumulative;
    }

    return total_days * seconds_per_day;
}

unsigned long long datetime::seconds_in_range(int start_year, int end_year) {
    unsigned long long total_seconds = 0;
    const int seconds_per_day = 86400;

    for (int year = start_year; year <= end_year; year++) {
        if (is_leap_year(year)) {
            total_seconds += 366LL * seconds_per_day;
        } else {
            total_seconds += 365LL * seconds_per_day;
        }
    }
    return total_seconds;
}

int datetime::day_of_week(int day, int month, int year) {
    if (month < 3) {
        month += 12;
        year--;
    }
    int K = year % 100;
    int J = year / 100;
    int h = (day + 13 * (month + 1) / 5 + K + K / 4 + J / 4 + 5 * J) % 7;
    return (h + 6) % 7;
}

datetime datetime::end_of_the_year() const {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);
    return datetime(31, 12, pack.year, 23, 59, 59, 999999);
}

datetime datetime::end_of_the_week() const {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);

    int dow = day_of_week();   // 0=Sunday, ...,6=Saturday
    int days_to_add = 6 - dow; // Days until Saturday

    // Calculate the new day, month, and year
    int new_day = pack.day + days_to_add;
    int new_month = pack.month;
    int new_year = pack.year;

    // Handle month overflow
    while (new_day > days_in_month[new_month - 1] + ((new_month == 2 && is_leap_year(new_year)) ? 1 : 0)) {
        new_day -= days_in_month[new_month - 1] + ((new_month == 2 && is_leap_year(new_year)) ? 1 : 0);
        new_month++;
        if (new_month > 12) {
            new_month = 1;
            new_year++;
        }
    }

    // Set time to 23:59:59.999999
    return datetime(new_day, new_month, new_year, 23, 59, 59, 999999);
}

datetime datetime::begin_of_the_month() const {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);
    return datetime(1, pack.month, pack.year, 0, 0, 0, 0);
}

datetime datetime::begin_of_the_year() const {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);
    return datetime(1, 1, pack.year, 0, 0, 0, 0);
}

datetime datetime::begin_of_the_week() const {
    datetime_struct pack;
    epoch_to_datetime_pack(data, pack);

    int dow = day_of_week(); // 0=Sunday, ...,6=Saturday
    int days_to_sub = dow;   // Days until Sunday

    // Calculate the new day, month, and year
    int new_day = pack.day - days_to_sub;
    int new_month = pack.month;
    int new_year = pack.year;

    // Handle month overflow
    while (new_day <= 0) {
        new_month--;
        if (new_month == 0) {
            new_month = 12;
            new_year--;
        }
        new_day += days_in_month[new_month - 1] + ((new_month == 2 && is_leap_year(new_year)) ? 1 : 0);
    }

    // Set time to 00:00:00.000000
    return datetime(new_day, new_month, new_year, 0, 0, 0, 0);
}

#ifdef HAS_STD_CHRONO
datetime datetime::now() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
#endif
}; // namespace gtr

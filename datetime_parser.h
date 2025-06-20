#ifndef DATETIME_PARSER_H
#define DATETIME_PARSER_H
#include "datetime.h"
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4244)
#endif
namespace gtr {

constexpr inline bool is_numeric(char a) { return a >= '0' && a <= '9'; }

constexpr inline void end_string(char *s) { *s = '\0'; }
constexpr const char *datetime_month_abbrev[]{"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

inline int datetime_get_month_from_sum(int sum) {
    static constexpr const int char_sum[] = {281, 269, 288, 291, 295, 301, 299, 401, 296, 294, 307, 268};
    int month = 1;
    while (char_sum[month - 1] != sum) month++;
    return month;
}

inline void datetime_strcpy(char *dest, const char *source) {
    do {
        *dest++ = *source++;
    } while (*source != '\0');
}

inline int datetime_atoi(const char *buffer) {
    bool neg = false;
    if (buffer[0] == '-') {
        neg = true;
        buffer++;
    }
    int value = 0;
    while (*buffer != '\0' && is_numeric(*buffer)) value = value * 10 + *buffer++ - '0';
    return neg ? -value : value;
}

inline int datetime_digits(int number) {
    if (number < 0) [[unlikely]] number = -number;
    if (number < 10) return 1;
    if (number < 100) return 2;
    if (number < 1000) return 3;
    if (number < 10000) return 4;
    if (number < 100000) return 5;
    if (number < 1000000) return 6;
    if (number < 10000000) return 7;
    if (number < 100000000) return 8;
    if (number < 1000000000) return 9;
    return 10;
}

inline void datetime_put_hour(char *dest, int digits, int number) {
    if (number < 10) {
        // Trailing zero
        if (digits == 2) {
            *dest++ = '0';
        }
        *dest++ = char('0' + number);
    } else {
        *dest++ = char('0' + number / 10);
        if (digits >= 2) {
            *dest++ = char('0' + number % 10);
        }
    }
    end_string(dest);
}

inline void datetime_put_month(char *dest, int digits, int number) { return datetime_put_hour(dest, digits, number); }

inline void datetime_put_day(char *dest, int digits, int number) { return datetime_put_hour(dest, digits, number); }

inline void datetime_put_minute(char *dest, int digits, int number) { return datetime_put_hour(dest, digits, number); }

inline void datetime_put_second(char *dest, int digits, int number) { return datetime_put_hour(dest, digits, number); }

static constexpr int pow10_table[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};

inline void datetime_put_year(char *dest, int digits, int number) {
    if (number < 0) {
        *dest++ = '-';
        number = -number;
    }

    int year_digits = datetime_digits(number);
    int start = year_digits - digits;
    if (start < 0) {
        start = 0;
    }
    for (int i = start; i < year_digits; i++) {
        int div = pow10_table[year_digits - i - 1];
        *dest++ = char('0' + number / div);
        number %= div;
    }
    end_string(dest);
}

inline void datetime_put_microsecond(char *dest, int digits, int number) {
    int microsecond_digits = datetime_digits(number);
    int digits_to_write = digits;
    if (microsecond_digits < 6) {
        // Trailing zeros
        for (int i = 0; i < 6 - microsecond_digits && i < digits_to_write; i++) {
            *dest++ = '0';
            digits_to_write--;
        }
    }
    for (int i = 0; i < digits_to_write; i++) {
        int div = pow10_table[microsecond_digits - i - 1];
        *dest++ = static_cast<char>('0' + number / div);
        number %= div;
    }
    end_string(dest);
}

enum year_format { year_four, year_two, year_all };
template <year_format Format = year_format::year_four> struct year_field {
    static inline int parse(const char **state, datetime_struct &pack) {
        char buffer[8] = {};
        int index = 0;
        while (is_numeric(**state)) {
            buffer[index++] = *(*state)++;
        }
        buffer[index] = '\0';
        pack.year = datetime_atoi(buffer);
        return index;
    }

    // Puts using format
    static inline void puts(const char **format, char **out, datetime_struct &pack) {
        if (*(++*format) == 'Y') {
            // Four digits
            if (*(++*format) == 'Y') {
                *format += 2;
                datetime_put_year(*out, 4, pack.year);
                *out += 4 + (pack.year < 0);
            }
            // Two digits
            else {
                datetime_put_year(*out, 2, pack.year);
                *out += 2 + (pack.year < 0);
            }

        }
        // All digits
        else if (**format == 'F') {
            int digits = datetime_digits(pack.year);
            datetime_put_year(*out, digits, pack.year);
            *out += digits + (pack.year < 0);
            (*format)++;
        }
    }

    // Puts using template argument
    static inline void puts(char **out, datetime_struct &pack) {
        if constexpr (Format == year_four) {
            datetime_put_year(*out, 4, pack.year);
            *out += 4 + (pack.year < 0);
        } else if constexpr (Format == year_two) {
            datetime_put_year(*out, 2, pack.year);
            *out += 2 + (pack.year < 0);
        }

        else {
            int digits = datetime_digits(pack.year);
            datetime_put_year(*out, digits, pack.year);
            *out += digits + (pack.year < 0);
        }
    }
};

struct day_field {
    static inline int parse(const char **state, datetime_struct &pack) {
        char buffer[8];
        buffer[0] = *(*state)++;
        buffer[1] = *(*state)++;
        end_string(&buffer[2]);
        pack.day = datetime_atoi(buffer);
        return 2;
    }

    // Puts using format
    static inline void puts(const char **format, char **out, datetime_struct &pack) {
        *format += 2;
        datetime_put_day(*out, 2, pack.day);
        *out += 2;
    }

    // Puts two using two digits
    static inline void puts(char **out, datetime_struct &pack) {
        datetime_put_day(*out, 2, pack.day);
        *out += 2;
    }
};

template <month_format Format = month_format::month_digits> struct month_field {
    static inline int parse(const char **state, datetime_struct &pack);
    static inline void puts(const char **format, char **out, datetime_struct &pack);
    static inline void puts(char **out, datetime_struct &pack);
};

template <month_format Format> inline void month_field<Format>::puts(const char **format, char **out, datetime_struct &pack) {
    datetime_strcpy(*out, datetime_month_abbrev[pack.month - 1]);
    *format += 3;
    *out += 3;
}

template <> inline void month_field<month_format::month_digits>::puts(const char **format, char **out, datetime_struct &pack) {
    datetime_put_month(*out, 2, pack.month);
    *format += 2;
    *out += 2;
}

template <month_format Format> inline void month_field<Format>::puts(char **out, datetime_struct &pack) {
    datetime_strcpy(*out, datetime_month_abbrev[pack.month - 1]);
    *out += 3;
}

template <> inline void month_field<month_format::month_digits>::puts(char **out, datetime_struct &pack) {
    datetime_put_month(*out, 2, pack.month);
    *out += 2;
}

template <> inline int month_field<month_format::month_digits>::parse(const char **state, datetime_struct &pack) {
    char buffer[8];
    buffer[0] = *(*state)++;
    buffer[1] = *(*state)++;
    end_string(&buffer[2]);
    pack.month = datetime_atoi(buffer);
    return 2;
}

template <month_format Format> inline int month_field<Format>::parse(const char **state, datetime_struct &pack) {
    pack.month = datetime_get_month_from_sum((*state)[0] + (*state)[1] + (*state)[2]);
    (*state) += 3;
    return 3;
}

struct hour_field {
    static inline int parse(const char **state, datetime_struct &pack) {
        char buffer[8];
        buffer[0] = *(*state)++;
        buffer[1] = *(*state)++;
        end_string(&buffer[2]);
        pack.hour = datetime_atoi(buffer);
        return 2;
    }

    // Puts using format
    static inline void puts(const char **format, char **out, datetime_struct &pack) {
        *format += 2;
        datetime_put_hour(*out, 2, pack.hour);
        *out += 2;
    }

    // Puts two using two digits
    static inline void puts(char **out, datetime_struct &pack) {
        datetime_put_hour(*out, 2, pack.hour);
        *out += 2;
    }
};

struct minute_field {
    static inline int parse(const char **state, datetime_struct &pack) {
        char buffer[8];
        buffer[0] = *(*state)++;
        buffer[1] = *(*state)++;
        end_string(&buffer[2]);
        pack.minute = datetime_atoi(buffer);
        return 2;
    }

    // Puts using format
    static inline void puts(const char **format, char **out, datetime_struct &pack) {
        *format += 2;
        datetime_put_minute(*out, 2, pack.minute);
        *out += 2;
    }

    // Puts two using two digits
    static inline void puts(char **out, datetime_struct &pack) {
        datetime_put_minute(*out, 2, pack.minute);
        *out += 2;
    }
};

struct second_field {
    static inline int parse(const char **state, datetime_struct &pack) {
        char buffer[8];
        buffer[0] = *(*state)++;
        buffer[1] = *(*state)++;
        end_string(&buffer[2]);
        pack.second = datetime_atoi(buffer);
        return 2;
    }

    // Puts using format
    static inline void puts(const char **format, char **out, datetime_struct &pack) {
        *format += 2;
        datetime_put_second(*out, 2, pack.second);
        *out += 2;
    }

    // Puts two using two digits
    static inline void puts(char **out, datetime_struct &pack) {
        datetime_put_second(*out, 2, pack.second);
        *out += 2;
    }
};

template <int Digits = 1> struct microsecond_field {
    static inline int parse(const char **state, datetime_struct &pack) {
        char buffer[8];
        int index = 0;
        while (is_numeric(**state)) {
            buffer[index++] = *(*state)++;
        }
        end_string(&buffer[index]);
        pack.microsecond = datetime_atoi(buffer);
        for (int i = 0; i < 6 - index; i++) {
            pack.microsecond *= 10;
        };
        return index;
    }

    // Puts using template arguments
    static inline void puts(char **out, datetime_struct &pack) {
        datetime_put_microsecond(*out, Digits, pack.microsecond);
        *out += Digits;
    }

    // Puts using format
    static inline void puts(const char **format, char **out, datetime_struct &pack) {
        int digits = 1;
        while (*(++*format) == 'z') {
            digits++;
        }
        datetime_put_microsecond(*out, digits, pack.microsecond);
        *out += digits;
    }
};

template <int Count = 1, char Sep = ':'> struct separator_field {
    static inline int parse(const char **state, datetime_struct &pack) {
        (void)pack;
        (*state) += Count;
        return Count;
    }
    // Puts using template arguments
    static inline void puts(char **out, datetime_struct &pack) {
        (void)pack;
        for (int i = 0; i < Count; i++) {
            *(*out)++ = Sep;
        }
    }
    // Puts using format
    static inline void puts(const char **format, char **out, datetime_struct &pack) {
        (void)pack;
        for (int i = 0; i < Count; i++) *(*out)++ = *(*format)++;
    }
};

#ifdef DATETIME_PERFECT_PARSER
template <class... Args> struct perfect_parser {
    static datetime parse_datetime(const char *date) {
        datetime_struct pack{};
        const char *state = date;
        parse_impl(&state, pack);
        return datetime{pack.day, pack.month, pack.year, pack.hour, pack.minute, pack.second, pack.microsecond};
    }

    static void put_datetime(datetime date, char *out) {
        datetime_struct pack;
        date.to_pack(pack);
        char *out_ptr = out;
        put_impl(&out_ptr, pack);
        end_string(out_ptr);
    }

  private:
    static void parse_impl(const char **state, datetime_struct &pack) { ((void)Args{}.parse(state, pack), ...); }
    static void put_impl(char **out, datetime_struct &pack) { (Args{}.puts(out, pack), ...); }
};

// Parses DD/MM/YYYY hh:mm:ss
using perfect_parser_default =
    perfect_parser<day_field, separator_field<>, month_field<>, separator_field<>, year_field<>, separator_field<>, hour_field,
                   separator_field<>, minute_field, separator_field<>, second_field>;
#endif // DATETIME_PERFERCT_PARSER
}; // namespace gtr

#ifdef _WIN32
#pragma warning(pop)
#endif
#endif

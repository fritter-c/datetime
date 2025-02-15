#ifndef DATETIME_PARSER_H
#define DATETIME_PARSER_H
#include "datetime.h"
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4244)
#endif
namespace gtr {

constexpr inline bool is_numeric(char a) {
    return a >= '0' && a <= '9';
}

constexpr inline void end_string(char *s) {
    *s = '\0';
}
constexpr const char *datetime_month_abbrev[]{"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

inline int datetime_get_month_from_sum(int sum) {
    constexpr const int char_sum[] = {281, 269, 288, 291, 295, 301, 299, 401, 296, 294, 307, 268};
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

constexpr int datetime_digits(int number) {
    if (number < 0)
        number = -number;
    if (number < 10)
        return 1;
    if (number < 100)
        return 2;
    if (number < 1000)
        return 3;
    if (number < 10000)
        return 4;
    if (number < 100000)
        return 5;
    if (number < 1000000)
        return 6;
    return -1;
}

inline void datetime_puts_integer(char *dest, int digits, int number) {
    bool neg = number < 0;
    *(dest += digits + neg) = '\0';
    number = neg ? -number : number;
    while (digits--) {
        *--dest = (number % 10) + '0';
        number /= 10;
    }
    if (neg)
        *--dest = '-';
}

struct year_field {
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

    static inline void puts(const char **format, char **out, datetime_struct &pack) {
        if (*(++*format) == 'Y') {
            // Four digits
            if (*(++*format) == 'Y') {
                *format += 2;
                datetime_puts_integer(*out, 4, pack.year);
                *out += 4 + (pack.year < 0);
            }
            // Two digits
            else {
                int digits = datetime_digits(pack.year);
                int year = pack.year;
                int factor = 1;
                for (; digits - 2 > 0; digits--) {
                    factor *= 10;
                }
                datetime_puts_integer(*out, 2, year % factor);
                *out += 2 + (pack.year < 0);
            }
        }
        // All digits
        else if (**format == 'F') {
            int digits = datetime_digits(pack.year);
            datetime_puts_integer(*out, digits, pack.year);
            *out += digits + (pack.year < 0);
            (*format)++;
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
    static inline void puts(const char **format, char **out, datetime_struct &pack) {
        *format += 2;
        datetime_puts_integer(*out, 2, pack.day);
        *out += 2;
    }
};

template <month_format Format = month_format::month_digits> struct month_field {
    static inline int parse(const char **state, datetime_struct &pack);
    static inline void puts(const char **format, char **out, datetime_struct &pack);
};

template <month_format Format> inline void month_field<Format>::puts(const char **format, char **out, datetime_struct &pack) {
    datetime_strcpy(*out, datetime_month_abbrev[pack.month - 1]);
    *format += 3;
    *out += 3;
}

template <> inline void month_field<month_format::month_digits>::puts(const char **format, char **out, datetime_struct &pack) {
    datetime_puts_integer(*out, 2, pack.month);
    *format += 2;
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
    static inline void puts(const char **format, char **out, datetime_struct &pack) {
        *format += 2;
        datetime_puts_integer(*out, 2, pack.hour);
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
    static inline void puts(const char **format, char **out, datetime_struct &pack) {
        *format += 2;
        datetime_puts_integer(*out, 2, pack.minute);
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
    static inline void puts(const char **format, char **out, datetime_struct &pack) {
        *format += 2;
        datetime_puts_integer(*out, 2, pack.second);
        *out += 2;
    }
};

struct microsecond_field {
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
    static inline void puts(const char **format, char **out, datetime_struct &pack) {
        int digits = 1;
        while (*(++*format) == 'z') {
            digits++;
        }
        datetime_puts_integer(*out, digits, pack.microsecond);
        *out += digits;
    }
};

template <int Count = 1> struct separator_field {
    static inline int parse(const char **state, datetime_struct &pack) {
        (void)pack;
        (*state) += Count;
        return Count;
    }
    static inline void puts(const char **format, char **out, datetime_struct &pack) {
        (void)pack;
        *(*out)++ = *(*format)++;
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

    static void put_datetime(datetime date, const char *format, char *out) {
        datetime_struct pack;
        date.to_pack(pack);
        const char *state = format;
        char *out_ptr = out;
        put_impl(&state, &out_ptr, pack);
        end_string(out_ptr);
    }

  private:
    static void parse_impl(const char **state, datetime_struct &pack) { ((void)Args{}.parse(state, pack), ...); }
    static void put_impl(const char **format, char **out, datetime_struct &pack) { (Args{}.puts(format, out, pack), ...); }
};

// Parses DD/MM/YYYY hh:mm:ss
using perfect_parser_default = perfect_parser<day_field, separator_field<>, month_field<>, separator_field<>, year_field, separator_field<>,
                                              hour_field, separator_field<>, minute_field, separator_field<>, second_field>;
#endif // DATETIME_PERFERCT_PARSER
};     // namespace gtr

#ifdef _WIN32
#pragma warning(pop)
#endif
#endif

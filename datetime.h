#ifndef GTR_DATETIME_H
#define GTR_DATETIME_H

constexpr auto DATETIME_DEFAULT_FORMAT = "DD/MM/YYYY hh:mm:ss";
constexpr auto DATETIME_INVALID = -9223372036854775807LL - 1LL;
#define _IS_ALPHA(A) ((((A) >= '0') && ((A) <= '9')))
#define _END_STRING(A) ((*(A)) = '\0')

namespace gtr {
constexpr const char *datetime_month_abbrev[]{"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                              "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

enum class date_format { text_date, iso_date };

enum class month_format { month_digits, month_abbrev };

struct datetime_pack {
    int year;
    unsigned char month;
    unsigned char day;

    unsigned char hour;
    unsigned char minute;
    int second;
    int microsecond;
};

struct datetime {
    long long data;

    inline constexpr datetime() : data(0){};
    inline constexpr datetime(long long _data) : data(_data){};

    datetime(int day, int month, int year, int hour = 0, int minute = 0, int second = 0,
             int microsecond = 0);
    datetime(const char *date, const char *format = DATETIME_DEFAULT_FORMAT,
             date_format group_format = date_format::text_date);

    // trivial
    ~datetime() = default;
    datetime(const datetime &other) = default;
    datetime(datetime &&other) noexcept = default;
    datetime &operator=(const datetime &other) = default;
    datetime &operator=(datetime &&other) noexcept = default;

    inline bool is_valid() const { return data != DATETIME_INVALID; }

    bool to_string_format(char *out, const char *format = DATETIME_DEFAULT_FORMAT,
                          date_format group_format = date_format::text_date) const;

    bool from_string(const char *date, const char *format = DATETIME_DEFAULT_FORMAT,
                     date_format group_format = date_format::text_date);

    void to_pack(datetime_pack &) const;

    inline void from_pack(datetime_pack &pack) {
        *this = datetime(pack.day, pack.month, pack.year, pack.hour, pack.minute, pack.second,
                         pack.microsecond);
    }

    inline constexpr void add_microseconds(long long microseconds) { data += microseconds; }

    inline constexpr void add_seconds(long long seconds) { data += seconds * 1000000LL; }

    inline constexpr void add_minutes(int minutes) { data += minutes * 60LL * 1000000LL; }

    inline constexpr void add_hours(int hours) { data += hours * 60LL * 60LL * 1000000LL; }

    inline constexpr void add_days(int days) { data += days * 86400 * 1000000LL; }

    void add_months(int months);

    void add_years(int years);

    int day() const;

    int month() const;

    int year() const;

    int second() const;

    int minute() const;

    int hour() const;

    int microsecond() const;

    inline bool operator==(datetime other) const { return data == other.data; }

    inline bool operator!=(datetime other) const { return data != other.data; }

    inline bool operator>(datetime other) const { return data > other.data; }

    inline bool operator<(datetime other) const { return data < other.data; }

    inline bool operator>=(datetime other) const { return data >= other.data; }

    inline bool operator<=(datetime other) const { return data <= other.data; }

    inline datetime operator+(datetime other) const { return data + other.data; }

    inline datetime operator+=(datetime other) {
        data += other.data;
        return *this;
    }

    inline datetime operator-(datetime other) const { return data - other.data; }

    inline datetime operator-=(datetime other) {
        data -= other.data;
        return *this;
    }
};

inline int datetime_get_month_from_sum(int sum) {
    constexpr const int char_sum[] = {281, 269, 288, 291, 295, 301, 299, 401, 296, 294, 307, 268};
    int month = 1;
    while (char_sum[month - 1] != sum)
        month++;
    return month;
}

inline void datetime_strcpy(char *dest, const char *source) {
    do {
        *dest++ = *source++;
    } while (*source != '\0');
}

inline long long datetime_atoi(const char *buffer) {
    bool neg = false;
    if (buffer[0] == '-') {
        neg = true;
        buffer++;
    }
    long long value = 0;
    while (*buffer != '\0' && _IS_ALPHA(*buffer))
        value = value * 10 + *buffer++ - '0';
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
    static inline int parse(const char **state, datetime_pack &pack) {
        char buffer[8] = {};
        int index = 0;
        while (_IS_ALPHA(**state)) {
            buffer[index++] = *(*state)++;
        }
        buffer[index] = '\0';
        pack.year = datetime_atoi(buffer);
        return index;
    }

    static inline void puts(const char **format, char **out, datetime_pack &pack) {
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
    static inline int parse(const char **state, datetime_pack &pack) {
        char buffer[8];
        buffer[0] = *(*state)++;
        buffer[1] = *(*state)++;
        _END_STRING(&buffer[2]);
        pack.day = datetime_atoi(buffer);
        return 2;
    }
    static inline void puts(const char **format, char **out, datetime_pack &pack) {
        *format += 2;
        datetime_puts_integer(*out, 2, pack.day);
        *out += 2;
    }
};

template <month_format Format = month_format::month_digits> struct month_field {
    static inline int parse(const char **state, datetime_pack &pack) {
        if constexpr (Format == month_format::month_digits) {
            char buffer[8];
            buffer[0] = *(*state)++;
            buffer[1] = *(*state)++;
            _END_STRING(&buffer[2]);
            pack.month = datetime_atoi(buffer);
            return 2;
        } else {
            pack.month = datetime_get_month_from_sum((*state)[0] + (*state)[1] + (*state)[2]);
            (*state) += 3;
            return 3;
        }
    }
    static inline void puts(const char **format, char **out, datetime_pack &pack) {
        if constexpr (Format == month_format::month_abbrev) {
            datetime_strcpy(*out, datetime_month_abbrev[pack.month - 1]);
            *format += 3;
            *out += 3;
        } else {
            datetime_puts_integer(*out, 2, pack.month);
            *format += 2;
            *out += 2;
        }
    }
};

struct hour_field {
    static inline int parse(const char **state, datetime_pack &pack) {
        char buffer[8];
        buffer[0] = *(*state)++;
        buffer[1] = *(*state)++;
        _END_STRING(&buffer[2]);
        pack.hour = datetime_atoi(buffer);
        return 2;
    }
    static inline void puts(const char **format, char **out, datetime_pack &pack) {
        *format += 2;
        datetime_puts_integer(*out, 2, pack.hour);
        *out += 2;
    }
};
struct minute_field {
    static inline int parse(const char **state, datetime_pack &pack) {
        char buffer[8];
        buffer[0] = *(*state)++;
        buffer[1] = *(*state)++;
        _END_STRING(&buffer[2]);
        pack.minute = datetime_atoi(buffer);
        return 2;
    }
    static inline void puts(const char **format, char **out, datetime_pack &pack) {
        *format += 2;
        datetime_puts_integer(*out, 2, pack.minute);
        *out += 2;
    }
};

struct second_field {
    static inline int parse(const char **state, datetime_pack &pack) {
        char buffer[8];
        buffer[0] = *(*state)++;
        buffer[1] = *(*state)++;
        _END_STRING(&buffer[2]);
        pack.second = datetime_atoi(buffer);
        return 2;
    }
    static inline void puts(const char **format, char **out, datetime_pack &pack) {
        *format += 2;
        datetime_puts_integer(*out, 2, pack.second);
        *out += 2;
    }
};

struct microsecond_field {
    static inline int parse(const char **state, datetime_pack &pack) {
        char buffer[8];
        int index = 0;
        while (_IS_ALPHA(**state)) {
            buffer[index++] = *(*state)++;
        }
        _END_STRING(&buffer[index]);
        pack.microsecond = datetime_atoi(buffer);
        for (int i = 0; i < 6 - index; i++) {
            pack.microsecond *= 10;
        };
        return index;
    }
    static inline void puts(const char **format, char **out, datetime_pack &pack) {
        int digits = 1;
        while (*(++*format) == 'z') {
            digits++;
        }
        datetime_puts_integer(*out, digits, pack.microsecond);
        *out += digits;
    }
};

template <int Count = 1> struct separator_field {
    static inline int parse(const char **state, datetime_pack &pack) {
        (void)pack;
        (*state) += Count;
        return Count;
    }
    static inline void puts(const char **format, char **out, datetime_pack &pack) {
        (void)pack;
        *(*out)++ = *(*format)++;
    }
};
#ifdef DATETIME_PERFECT_PARSER
template <class... Args> struct perfect_parser {
    static datetime parse_datetime(const char *date) {
        datetime_pack pack;
        const char *state = date;
        parse_impl(&state, pack);
        return datetime{pack.day,    pack.month,  pack.year,       pack.hour,
                        pack.minute, pack.second, pack.microsecond};
    }

    static void put_datetime(datetime date, const char *format, char *out) {
        datetime_pack pack;
        date.to_pack(pack);
        const char *state = format;
        char *out_ptr = out;
        put_impl(&state, &out_ptr, pack);
        _END_STRING(out_ptr);
    }

  private:
    static void parse_impl(const char **state, datetime_pack &pack) {
        ((void)Args{}.parse(state, pack), ...);
    }
    static void put_impl(const char **format, char **out, datetime_pack &pack) {
        (Args{}.puts(format, out, pack), ...);
    }
};

// Parses DD/MM/YYYY hh:mm:ss
using perfect_parser_default =
    perfect_parser<day_field, separator_field<>, month_field<>, separator_field<>, year_field,
                   separator_field<>, hour_field, separator_field<>, minute_field,
                   separator_field<>, second_field>;
#endif // DATETIME_PERFERCT_PARSER
};     // namespace gtr
#endif

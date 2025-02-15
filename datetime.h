#ifndef GTR_DATETIME_H
#define GTR_DATETIME_H

namespace gtr {

/**
 * @brief The default format for datetime strings.
 */
constexpr auto DATETIME_DEFAULT_FORMAT = "DD/MM/YYYY hh:mm:ss";

/**
 * @brief The value representing an invalid datetime.
 */
constexpr auto DATETIME_INVALID = -9223372036854775807LL - 1LL;

/**
 * @brief The format options for date representation.
 */
enum class date_format { text_date, iso_date };

/**
 * @brief The format options for month representation.
 */
enum class month_format { month_digits, month_abbrev };

/**
 * @brief A structure representing the components of a datetime.
 */
struct datetime_struct {
    int year;             /**< The year component of the datetime. */
    unsigned char month;  /**< The month component of the datetime. */
    unsigned char day;    /**< The day component of the datetime. */
    unsigned char hour;   /**< The hour component of the datetime. */
    unsigned char minute; /**< The minute component of the datetime. */
    int second;           /**< The second component of the datetime. */
    int microsecond;      /**< The microsecond component of the datetime. */

    long long to_datetime();
};

/**
 * @brief A class representing a datetime.
 * Holds the microseconds count since epoch in UTC+0
 */
struct datetime {
    long long data; /**< The internal representation of the datetime. */

    /**
     * @brief Default constructor.
     */
    inline constexpr datetime() : data(0) {};

    /**
     * @brief Constructor that initializes the datetime with the given value.
     * @param _data The value to initialize the datetime with.
     */
    inline constexpr datetime(long long _data) : data(_data) {};

    /**
     * @brief Constructor that creates a datetime from the given components.
     * @param day The day component of the datetime.
     * @param month The month component of the datetime.
     * @param year The year component of the datetime.
     * @param hour The hour component of the datetime. Default is 0.
     * @param minute The minute component of the datetime. Default is 0.
     * @param second The second component of the datetime. Default is 0.
     * @param microsecond The microsecond component of the datetime. Default is 0.
     */
    datetime(int day, int month, int year, int hour = 0, int minute = 0, int second = 0, int microsecond = 0);

    /**
     * @brief Constructor that creates a datetime from the given string representation.
     * @param date The string representation of the datetime.
     * @param format The format of the datetime string. Default is DATETIME_DEFAULT_FORMAT.
     * @param group_format The format of the date component in the datetime string. Default is date_format::text_date.
     */
    datetime(const char *date, const char *format = DATETIME_DEFAULT_FORMAT, date_format group_format = date_format::text_date);

    // trivial
    ~datetime() = default;
    datetime(const datetime &other) = default;
    datetime(datetime &&other) noexcept = default;
    datetime &operator=(const datetime &other) = default;
    datetime &operator=(datetime &&other) noexcept = default;

    /**
     * @brief Checks if the datetime is valid.
     * @return True if the datetime is valid, false otherwise.
     */
    inline bool is_valid() const { return data != DATETIME_INVALID; }

    /**
     * @brief Converts the datetime to a string representation.
     * @param out The output buffer to store the string representation.
     * @param format The format of the datetime string. Default is DATETIME_DEFAULT_FORMAT.
     * @param group_format The format of the date component in the datetime string. Default is date_format::text_date.
     * @return True if the conversion is successful, false otherwise.
     */
    bool to_string_format(char *out, const char *format = DATETIME_DEFAULT_FORMAT, date_format group_format = date_format::text_date) const;

    /**
     * @brief Converts a string representation to a datetime.
     * @param date The string representation of the datetime.
     * @param format The format of the datetime string. Default is DATETIME_DEFAULT_FORMAT.
     * @param group_format The format of the date component in the datetime string. Default is date_format::text_date.
     * @return True if the conversion is successful, false otherwise.
     */
    bool from_string(const char *date, const char *format = DATETIME_DEFAULT_FORMAT, date_format group_format = date_format::text_date);

    /**
     * @brief Converts the datetime to a datetime_pack structure.
     * @param pack The datetime_pack structure to store the components of the datetime.
     */
    void to_pack(datetime_struct &pack) const;

    /**
     * @brief Converts the datetime to the specified timezone.
     * @param timezone The timezone to convert the datetime to.
     */
    void to_timezone(const char *timezone);

    /**
     * @brief Converts a datetime_pack structure to a datetime.
     * @param pack The datetime_pack structure containing the components of the datetime.
     */
    inline void from_pack(datetime_struct &pack) {
        *this = datetime(pack.day, pack.month, pack.year, pack.hour, pack.minute, pack.second, pack.microsecond);
    }

    /**
     * @brief Adds the specified number of microseconds to the datetime.
     * @param microseconds The number of microseconds to add.
     */
    inline constexpr void add_microseconds(long long microseconds) { data += microseconds; }

    /**
     * @brief Adds the specified number of seconds to the datetime.
     * @param seconds The number of seconds to add.
     */
    inline constexpr void add_seconds(long long seconds) { data += seconds * 1000000LL; }

    /**
     * @brief Adds the specified number of minutes to the datetime.
     * @param minutes The number of minutes to add.
     */
    inline constexpr void add_minutes(int minutes) { data += minutes * 60LL * 1000000LL; }

    /**
     * @brief Adds the specified number of hours to the datetime.
     * @param hours The number of hours to add.
     */
    inline constexpr void add_hours(int hours) { data += hours * 60LL * 60LL * 1000000LL; }

    /**
     * @brief Adds the specified number of days to the datetime.
     * @param days The number of days to add.
     */
    inline constexpr void add_days(int days) { data += days * 86400 * 1000000LL; }

    /**
     * @brief Adds the specified number of months to the datetime.
     * @param months The number of months to add.
     */
    void add_months(int months);

    /**
     * @brief Adds the specified number of years to the datetime.
     * @param years The number of years to add.
     */
    void add_years(int years);

    /**
     * @brief Gets the day component of the datetime.
     * @return The day component of the datetime.
     */
    int day() const;

    /**
     * @brief Gets the month component of the datetime.
     * @return The month component of the datetime.
     */
    int month() const;

    /**
     * @brief Gets the year component of the datetime.
     * @return The year component of the datetime.
     */
    int year() const;

    /**
     * @brief Gets the second component of the datetime.
     * @return The second component of the datetime.
     */
    int second() const;

    /**
     * @brief Gets the minute component of the datetime.
     * @return The minute component of the datetime.
     */
    int minute() const;

    /**
     * @brief Gets the hour component of the datetime.
     * @return The hour component of the datetime.
     */
    int hour() const;

    /**
     * @brief Gets the microsecond component of the datetime.
     * @return The microsecond component of the datetime.
     */
    int microsecond() const;

    /**
     * Returns the minute of the day.
     * @return The minute of the day.
     */
    int get_minute_of_day() const;

    /**
     * Returns the second of the day.
     * @return The second of the day.
     */
    int get_second_of_day() const;

    /**
     * Returns the microsecond of the day.
     * @return The microsecond of the day.
     */
    int get_microsecond_of_day() const;

    /**
     * @brief Gets the day of the week for the datetime.
     * @return The day of the week for the datetime.
     */
    int day_of_week() const;

    /**
     * @brief Gets the day of the year for the datetime.
     * @return The day of the year for the datetime.
     */
    datetime date() const;

    /**
     * @brief Returns a datetime object representing the end of the month.
     * 
     * This function calculates and returns a datetime object that corresponds
     * to the last moment (23:59:59) of the last day of the current month.
     * 
     * @return datetime The datetime object representing the end of the month.
     */
    datetime end_of_the_month() const;

    /**
     * @brief Returns the datetime representing the beginning of the month.
     * 
     * This function calculates and returns a datetime object that corresponds
     * to the first day of the current month at 00:00:00.
     * 
     * @return datetime The datetime object representing the beginning of the month.
     */
    datetime begin_of_the_month() const;

    /**
     * @brief Returns the datetime representing the beginning of the month.
     * 
     * This function calculates and returns a datetime object that corresponds
     * to the first day of the current month at 00:00:00.
     * 
     * @return datetime The datetime object representing the beginning of the month.
     */
    datetime end_of_the_year() const;

    /**
     * @brief Returns the datetime representing the beginning of the year.
     * 
     * This function calculates and returns the datetime object that corresponds
     * to the first moment of the first day of the current year.
     * 
     * @return datetime The datetime object representing the beginning of the year.
     */
    datetime begin_of_the_year() const;
    
    /**
     * @brief Returns the datetime representing the end of the week.
     * 
     * This function calculates and returns the datetime object that corresponds
     * to the end of the current week. The end of the week is typically considered
     * to be the last moment of Sunday (23:59:59).
     * 
     * @return datetime The datetime object representing the end of the week.
     */
    datetime end_of_the_week() const;

    /**
     * @brief Returns the datetime representing the beginning of the week.
     * 
     * This function calculates and returns the datetime object that corresponds
     * to the beginning of the current week. The beginning of the week is typically
     * considered to be the first moment of Monday (00:00:00).
     * 
     * @return datetime The datetime object representing the beginning of the week.
     */
    datetime begin_of_the_week() const;

    /**
     * @brief Returns the datetime representing the beginning of the day.
     * 
     * This function calculates and returns the datetime object that corresponds
     * to the beginning of the current day. The beginning of the day is typically
     * considered to be the first moment of the day (00:00:00).
     * 
     * @return datetime The datetime object representing the beginning of the day.
     */
    datetime begin_of_the_day() const;

    /**
     * @brief Returns the datetime representing the end of the day.
     * 
     * This function calculates and returns the datetime object that corresponds
     * to the end of the current day. The end of the day is typically considered
     * to be the last moment of the day (23:59:59).
     * 
     * @return datetime The datetime object representing the end of the day.
     */
    datetime end_of_the_day() const;

    /**
     * @brief Checks if the datetime is on a different day than the given datetime.
     * 
     * This function compares the current datetime object with another datetime object
     * to determine if they represent different days.
     * 
     * @param other The datetime object to compare with.
     * @return True if the datetimes are on different days, false otherwise.
     */
    bool different_day(datetime other) const;

    /**
     * @brief Checks if the datetime is in a different month than the given datetime.
     * 
     * This function compares the current datetime object with another datetime object
     * to determine if they represent different months.
     * 
     * @param other The datetime object to compare with.
     * @return True if the datetimes are in different months, false otherwise.
     */
    bool different_month(datetime other) const;

    /**
     * @brief Checks if the datetime is in a different year than the given datetime.
     *
     * This function compares the current datetime object with another datetime object
     * to determine if they represent different years.
     *
     * @param other The datetime object to compare with.
     * @return True if the datetimes are in different years, false otherwise.
     */
    bool different_year(datetime other) const;

    /**
     * @brief Returns the amount of days in the month.
     * @return The amount of days in the month.
     */
    static int month_day_count(int month, int year);

    /**
     * @brief Returns the day of the week for the given date.
     * @return The day of the week for the given date.
     */
    static int day_of_week(int day, int month, int year);

    /**
     * @brief Equality comparison operator.
     * @param other The datetime to compare with.
     * @return True if the datetimes are equal, false otherwise.
     */
    inline bool operator==(datetime other) const { return data == other.data; }

    /**
     * @brief Inequality comparison operator.
     * @param other The datetime to compare with.
     * @return True if the datetimes are not equal, false otherwise.
     */
    inline bool operator!=(datetime other) const { return data != other.data; }

    /**
     * @brief Greater than comparison operator.
     * @param other The datetime to compare with.
     * @return True if this datetime is greater than the other datetime, false otherwise.
     */
    inline bool operator>(datetime other) const { return data > other.data; }

    /**
     * @brief Less than comparison operator.
     * @param other The datetime to compare with.
     * @return True if this datetime is less than the other datetime, false otherwise.
     */
    inline bool operator<(datetime other) const { return data < other.data; }

    /**
     * @brief Greater than or equal to comparison operator.
     * @param other The datetime to compare with.
     * @return True if this datetime is greater than or equal to the other datetime, false otherwise.
     */
    inline bool operator>=(datetime other) const { return data >= other.data; }

    /**
     * @brief Less than or equal to comparison operator.
     * @param other The datetime to compare with.
     * @return True if this datetime is less than or equal to the other datetime, false otherwise.
     */
    inline bool operator<=(datetime other) const { return data <= other.data; }

    /**
     * @brief Addition operator.
     * @param other The datetime to add.
     * @return The result of the addition.
     */
    inline datetime operator+(datetime other) const { return data + other.data; }

    /**
     * @brief Addition assignment operator.
     * @param other The datetime to add.
     * @return This datetime after the addition.
     */
    inline datetime operator+=(datetime other) {
        data += other.data;
        return *this;
    }

    /**
     * @brief Subtraction operator.
     * @param other The datetime to subtract.
     * @return The result of the subtraction.
     */
    inline datetime operator-(datetime other) const { return data - other.data; }

    /**
     * @brief Subtraction assignment operator.
     * @param other The datetime to subtract.
     * @return This datetime after the subtraction.
     */
    inline datetime operator-=(datetime other) {
        data -= other.data;
        return *this;
    }

#ifdef HAS_STD_CHRONO
    static datetime now();
#endif
};
} // namespace gtr
#endif

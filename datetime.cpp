#include "datetime.h"

namespace gtr {
/**
 * Months abbreviation
 */
    constexpr const char *datetime_month_abbrev[]{"Jan", "Feb", "Mar", "Apr",
                                                  "May", "Jun", "Jul", "Aug",
                                                  "Sep", "Oct", "Nov", "Dec"};

    constexpr inline static bool
    is_alpha(const char c){return (c >= '0' && c <= '9');}

/**
* For fasting parsing.
* @param sum of chars in the month abbreviation
* @return return the month 1-indexed
* @note: NOT SAFE for uncontrolled inputs
*/
    constexpr inline static
    int get_month_from_sum(int sum){
        constexpr const int char_sum[] = {
            281,  // Jan
            269,  // Feb
            288,  // Mar
            291,  // Apr
            295,  // May
            301,  // Jun
            299,  // Jul
            401,  // Aug
            296,  // Sep
            294,  // Oct
            307,  // Nov
            268   // Dec
        };
        int month = 1;
        while(char_sum[month - 1] != sum) month++;
        return month;
    }
    
/**
 * Get the numbers of digits for a given year (-999999 to 999999)
 * @param n The year
 * @return The number of digits or -1 if mod(n) > 999999
 */
    constexpr inline static int
    datetime_year_digits(int n){
        if (n < 0) n = -n;
        if (n < 10) return 1;
        if (n < 100) return 2;
        if (n < 1000) return 3;
        if (n < 10000) return 4;
        if (n < 100000) return 5;
        if (n < 1000000) return 6;
        // Not suported in microseconds precision
        return -1;
    }

/**
 * Positive atoi given dates are mostly positive
 * @param number The number string
 * @return number as an positive unsigned int
 */
    static inline constexpr unsigned int
    positive_atoi(const char* number){
        unsigned int result  = 0;
        while(*number != '\0') result = result * 10 + *number++ - '0';
        return result;
    }

/**
 * strcpy implementation
 * @param dest Destination string
 * @param data Source string
 */
    inline static void
    string_copy(char* dest, const char* data){
        do{*dest++ = *data;}while(*data++ != '\0');
    }

/**
 * Puts a positive integer of n digits in the buffer plus the null-terminator
 * @param dest Destination string
 * @param digits Number of digits
 * @param number Integer positive number
 * @attention dest should support 4 + 1 char
 */
    static inline void
    put_positive_number(char *dest, int digits,int number) {
        *(dest += digits) = '\0'; 
        for(int i= 0; i < digits; i++) {*--dest = (number % 10) + '0'; number /= 10;}
    }

/**
 * Puts a negative sign plus the negative integer of n digits in the buffer plus the null-terminator
 * @param dest Destination string
 * @param digits Number of digits
 * @param number Integer negative number
 * @attention dest should support 4 + 2 char
 */
    static inline void
    put_negative_number(char * dest, int digits, int number){
        *(dest+= digits +1) = '\0';
        number = -number;
        for(int i= 0; i < digits; i++) {*--dest = (number % 10) + '0'; number /= 10;}
        *--dest = '-';
    }
    constexpr unsigned int g_monthdays[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
    constexpr unsigned int g_lpmonthdays[13] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};

    static inline constexpr bool
    is_leap_year(const int year){return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));}

    static inline constexpr int
    year_lenght_in_days(const int year){return is_leap_year(year) ? g_lpmonthdays[12] : g_monthdays[12];}
    
    static inline constexpr unsigned int
    days_until_month(const int year,const int month){return (is_leap_year(year) ? g_lpmonthdays[month -1] : g_monthdays[month -1]);}

    static inline constexpr long long
    seconds_since_epoch(const int day, const int month,
                        const int year, const int hour,
                        const int minute, const int second) {

        //  Reference https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04
        int day_corrected = days_until_month(year, month) + day - 1;
        int year_corrected = year - 1900;
        return second + minute * 60 + hour * 3600 + day_corrected * 86400 +
            (year_corrected - 70) * 31536000 +
            ((year_corrected - 69)/4)*86400 -
            ((year_corrected - 1)/100)*86400 + ((year_corrected + 299)/400)*86400;
    }
    
    static inline constexpr void
    epoch_to_datetime_pack(long long time, datetime_pack &pack){

        // Set microseconds
        if (time > 0)
            pack.microsecond = static_cast<int>(time % 1000000LL);
        else
            pack.microsecond = static_cast<int>(-time % 1000000LL);
        
        time /= 1000000LL;

        // Adapted from sourceware NewLib 
        long long days = time / (24 * 60 * 60) + 719468L;
        long long sec_per_day = time % (24 * 60 * 60);
        if (sec_per_day < 0)
        {
            sec_per_day += 24 * 60 * 60;
            days--;
        }

        int era = (days >= 0 ? days : days - 146097L + 1) / 146097L;
        unsigned long long era_day = days - era * 146097L;
        unsigned int era_year = (era_day - era_day / (1461 - 1) + era_day / 36524L - era_day / (146097L - 1))/365;
        unsigned int year_day = era_day - (365 * era_year + era_year / 4 - era_year / 100);
        unsigned month = (5 * year_day + 2)/153;
        unsigned day = year_day - (153 * month + 2)/5 + 1;
        month += month < 10 ? 2 : -10; 
        int year = era_year + era * 400 + static_cast<int>(month <= 1);
        //Set the year
        pack.year = year;

        // Set the month and month day
        pack.month = month + 1;
        pack.day = day;

        // Set hour, minute and second
        pack.hour = sec_per_day / (60 * 60);
        pack.minute = (sec_per_day % (60 * 60)) / 60;
        pack.second = (sec_per_day % (60 * 60)) % 60;
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
 * - u to uuuuuu: One to six-digit microsecond (0 to 999999)
 * - Any other characters in the format string are copied directly to the output.
 *
 * Example format strings:
 * - "YYYY-MM-DD hh:mm:ss" -> "2024-07-22 14:35:45"
 * - "DD/MM/YYYY" -> "22/07/2024"
 * - "MA DD, YYYY" -> "Ju 22, 2024"
 * - "hh:mm:ss.uuuuuu" -> "14:35:45.123456"
 *
 * @param year The year component of the datetime.
 * @param month The month component of the datetime.
 * @param day The day component of the datetime.
 * @param hour The hour component of the datetime.
 * @param minute The minute component of the datetime.
 * @param second The second component of the datetime.
 * @param microsecond The microsecond component of the datetime.
 * @param out The output string buffer to hold the formatted datetime string.
 * @param error The output string buffer to hold any error messages.
 * @param format The format string specifying the datetime representation.
 * @return True if the conversion was successful, false otherwise.
 */
    static bool
    datetime_to_string(int year, int month, int day, int hour,
                       int minute, int second,
                       int microsecond, char* out, char* error,
                       const char *format = DATETIME_DEFAULT_FORMAT, date_format group_format = date_format::text_date) {
        if (group_format == date_format::text_date){
            const char* state = format;
            char* out_ptr = out;
            string_copy(error, "");
            while (*state != '\0') {
                switch (*state) {
                case 'D':
                    if (*(state + 1) == 'D') {
                        put_positive_number(out_ptr, 2, day);
                        state += 2;
                    } else {
                        string_copy(error, "Invalid day format (two digits required)"); 
                        *out_ptr = '\0';
                        return false;
                    }
                    out_ptr += 2;
                    break;
                case 'M':
                    if (*(state + 1) == 'M' && *(state + 2) == 'M'){
                        string_copy(out_ptr, datetime_month_abbrev[month -1]);
                        state+= 3;
                        out_ptr += 3;
                    }   
                    else if (*(state + 1) == 'M') {
                        put_positive_number(out_ptr, 2, month);
                        state += 2;
                        out_ptr += 2;
                    }                  
                    else {
                        string_copy(error,"Invalid month format (two digits required)");
                        *out_ptr = '\0';
                        return false;
                    }
                    break;
                case 'Y':
                    // Puts only 4 signed digits years (useful from 9999BC to 9999AC)
                    if (*(state + 1) == 'Y' && *(state + 2) == 'Y' && *(state + 3) == 'Y') {
                        if (year >= 0){
                            put_positive_number(out_ptr, 4, year);
                            out_ptr += 4;
                        }
                        else{
                            put_negative_number(out_ptr, 4, year);
                            out_ptr += 5;
                        }
                        state += 4;            
                    }
                    //Puts only 2 signed digits
                    else if (*(state + 1) == 'Y'){
                        int year_format = year % 100;
                        if (year_format >= 0){
                            put_positive_number(out_ptr, 2, year_format);
                            out_ptr += 2;
                        }
                        else{
                            put_negative_number(out_ptr, 2, year_format);
                            out_ptr += 3;
                        }
                        state += 2;
                    }
                    // Puts the whole signed number
                    else if (*(state + 1) == 'F'){
                        int digits = datetime_year_digits(year);
                        if (year > 0) 
                            put_positive_number(out_ptr, digits, year);
                        else
                            put_negative_number(out_ptr, digits++, year);
                        state += 2;
                        out_ptr += digits;
                    }
                    else {
                        string_copy(error,"Invalid year format (four digits required)");
                        *out_ptr = '\0';
                        return false;
                    }
                    break;
                case 'h':
                    if (*(state + 1) == 'h') {
                        put_positive_number(out_ptr, 2, hour);
                        state += 2;
                    } else {
                        string_copy(error,"Invalid hour format (two digits required)");
                        *out_ptr = '\0';
                        return false;
                    }
                    out_ptr += 2;
                    break;
                case 'm':
                    if (*(state + 1) == 'm') {
                        put_positive_number(out_ptr, 2, minute);
                        state += 2;
                    } else {
                        string_copy(error,"Invalid minute format (two digits required)");
                        *out_ptr = '\0';
                        return false;
                    }
                    out_ptr += 2;
                    break;
                case 's':
                    if (*(state + 1) == 's') {
                        put_positive_number(out_ptr, 2, second);
                        state += 2;
                    } else {
                        string_copy(error, "Invalid second format (two digits required)");
                        *out_ptr = '\0';
                        return false; 
                    }
                    out_ptr += 2;
                    break;
                case 'z':
                {
                    int digits = 1;
                    while(*++state == 'z') {digits++;}
                    put_positive_number(out_ptr, digits, microsecond);
                    out_ptr += digits;
                    break; 
                }
                default:                
                    *out_ptr++ = *state++;
                    break;
                }
            }
            *out_ptr = '\0';
            return true;
        }
        return datetime_to_string(year, month, day, hour, minute, second, microsecond, out, error, "YYYY-MM-DDThh:mm:ss+00:00", date_format::text_date);
    }

    static long long
    parse_datetime_string(const char * __restrict date, const char * __restrict format, date_format group_format = date_format::text_date) {
        int year{0}, month{0}, day{0}, hour{0}, minute{0}, second{0}, microsecond{0};
        const char* state = format;
        const char* date_char = date;
        if (group_format == date_format::text_date){
            while(*state != '\0'){
                char buffer[8];
                int index = 0;
                char init_state = *state;
                int* result = &day;
                bool separator = false;
                bool special = false;
                switch(*state){
                case 'M':
                    if (*(state +1) == 'M' && *(state + 2 ) == 'M'){
                        special = true;
                        int sum = *date_char + *(date_char + 1) + *(date_char + 2);
                        month = get_month_from_sum(sum);
                        state += 3;
                        date_char += 3;
                    }
                    result = &month;
                    break;
                case 'Y':
                    if (*(state + 1) == 'F'){
                        special = true;
                        while(is_alpha(*date_char))
                            buffer[index++] = *date_char++;
                        buffer[index] = '\0';
                        year = positive_atoi(buffer);
                    }
                    result = &year;
                    break;
                case 'D':
                    result = &day;
                    break;
                case 'h':
                    result = &hour;
                    break;
                case 'm':
                    result = &minute;
                    break;
                case 's':
                    result = &second;
                    break;
                case 'z':
                    result = &microsecond;
                    break;
                default:
                    separator = true;
                    break;
                }
                if (!separator && !special){
                    while(*state == init_state && *date_char != '\0')
                    {
                        buffer[index++] = *date_char++;
                        state++;
                    }
                    //Fill with zeros the rest in case no all 6 digits were provided
                    if (init_state == 'z')
                        while(index < 6) buffer[index++] = '0';
                                      
                    buffer[index] = '\0';
                    *result = positive_atoi(buffer);
                }
                else{
                    state++;
                    date_char++;
                }
            }
            return seconds_since_epoch(day, month, year, hour, minute,second) * 1000000LL + microsecond;
        }
        return parse_datetime_string(date, "YYYY-MM-DDThh:mm:ss+00:00", date_format::text_date);
    }
    
    datetime::datetime(int day, int month, int year, int hour, int minute, int second, int microsecond)
    {
        data = seconds_since_epoch(day, month, year, hour, minute,second) * 1000000LL + microsecond;
    }
    
    datetime::datetime(const char* date, const char* format, date_format group_format)
    {
        from_string(date, format, group_format);
    }

    bool
    datetime::from_string(const char* date, const char* format, date_format group_format)
    {
        data = parse_datetime_string(date, format, group_format);
        return data != DATETIME_INVALID;
    }

    bool
    datetime::to_string_format(char* out, char* error,const char* format, date_format group_format) const
    {
        datetime_pack pack;
        to_pack(pack);
        return datetime_to_string(pack.year, pack.month, pack.day, pack.hour, pack.minute, pack.second, pack.microsecond, out,error, format, group_format);
    }
    
    void
    datetime::add_days(int days)
    {
        datetime_pack pack;
        to_pack(pack);
        int current_year = pack.year;
        int current_year_day =  days_until_month(pack.year, pack.month) + pack.day - 1;
        int current_year_lenght = year_lenght_in_days(pack.year);
        if (current_year_day + days >= current_year_lenght){
            add_years(1);
            days -= (current_year_lenght - current_year_day);
            current_year_lenght = year_lenght_in_days(++current_year);
        }

        //Probably there is a faster way to do that but I'm not really smart
        while(days >= current_year_lenght){
            add_years(1);
            days -= current_year_lenght;
            current_year_lenght = year_lenght_in_days(++current_year);
        }
        data += days * 86400 * 1000000LL;
    }
    
    void
    datetime::add_months(int months)
    {
        datetime_pack pack;
        to_pack(pack);
        pack.month += months;

        if (pack.month > 12)
            add_years(pack.month / 12);

        pack.month %= 12;
        // Handle day overflow (e.g., adding one month to January 31 should result in February 28/29)
        static int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        if ((pack.year % 4 == 0 && pack.year % 100 != 0) || (pack.year % 400 == 0)) {
            days_in_month[1] = 29;  // February in a leap year
        }
        if (pack.day > days_in_month[pack.month - 1]) {
            pack.day = days_in_month[pack.month - 1];
        }

        *this = datetime(pack.day, pack.month, pack.year, pack.hour, pack.minute, pack.second, pack.microsecond);
    }

    void
    datetime::add_years(int years)
    {
        datetime_pack pack;
        to_pack(pack);
        pack.year += years;
        *this = datetime(pack.day, pack.month, pack.year, pack.hour, pack.minute, pack.second, pack.microsecond); 
    }
    
    void
    datetime::to_pack(datetime_pack& pack) const
    {
        epoch_to_datetime_pack(data, pack);
    }
};
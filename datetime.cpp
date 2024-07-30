#include "datetime.h"
#include "datetime_parser.h"
namespace gtr {
    constexpr inline bool is_leap_year(int year){return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;}
    constexpr unsigned int monthdays[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
    constexpr int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    static inline constexpr unsigned int
    days_until_month(const int year,const int month){return (is_leap_year(year) ? monthdays[month -1] + (month > 1): monthdays[month - 1]);}

    static inline constexpr unsigned int
    leap_years_count(int start_year, int end_year){
        start_year--;
        return ((end_year/4) - (end_year/100) + (end_year/400)) - ((start_year/4) - (start_year/100) + (start_year/400));
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
           
    static inline constexpr long long
    seconds_since_epoch(const int day, const int month,
                        const int year, const int hour,
                        const int minute, const int second) {

        // Reference https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04
        int day_corrected = days_until_month(year, month > 0 && month <= 12 ? month : 12) + day - 1;
        if (month == 2 && is_leap_year(year)){
            day_corrected--;
        }
        if (year >= 1970){
            long long year_corrected = year - 1900;
            return second + minute * 60 + hour * 3600 + day_corrected * 86400LL +
                (year_corrected - 70) * 31536000 +
                ((year_corrected - 69)/4)*86400 -
            ((year_corrected - 1)/100)*86400 + ((year_corrected + 299)/400)*86400;
        }
        int leaps = leap_years_count(year, 1970);
        if (year < 0 && is_leap_year(year))
            leaps++;
        int normal = (1970 - year) - leaps;
        int total_days = leaps * 366LL + normal * 365LL;
        total_days-= day_corrected;
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
    datetime_to_string(datetime date, char* out,
                       const char *format = DATETIME_DEFAULT_FORMAT, date_format group_format = date_format::text_date) {
        if (group_format == date_format::text_date){
            datetime_pack pack;
            date.to_pack(pack);
            const char* state = format;
            char* out_ptr = out;
            while (*state != '\0') {
                switch (*state) {
                case 'D':day_field::puts(&state, &out_ptr, pack); break;
                case 'M':
                    if(*(state + 1) == 'M' && *(state +2) == 'M')month_field<month_format::month_abbrev>::puts(&state,&out_ptr,pack);
                    else month_field<>::puts(&state,&out_ptr,pack);
                    break;
                case 'Y':year_field::puts(&state, &out_ptr, pack);break;
                case 'h':hour_field::puts(&state, &out_ptr, pack);break;
                case 'm':minute_field::puts(&state, &out_ptr, pack);break;
                case 's':second_field::puts(&state,&out_ptr, pack);break;
                case 'z':microsecond_field::puts(&state, &out_ptr, pack);break;
                default:separator_field<1>::puts(&state, &out_ptr, pack);break;
                }
            }
            end_string(out_ptr);
            return true;
        }
        return datetime_to_string(date, out, "YYYY-MM-DDThh:mm:ss+00:00", date_format::text_date);
    }

    static long long
    parse_datetime_string(const char * date, const char * format, date_format group_format = date_format::text_date) {
        const char* state = format;
        const char* date_char = date;
        datetime_pack pack{};
        if (group_format == date_format::text_date){
            while(*state != '\0'){
                switch(*state){
                case 'M':
                    if (*(state +1) == 'M' && *(state + 2 ) == 'M')state += month_field<month_format::month_abbrev>::parse(&date_char, pack);
                    else state += month_field<>::parse(&date_char, pack);
                    break;
                case 'Y':state += year_field::parse(&date_char, pack);break;
                case 'D':state += day_field::parse(&date_char, pack);break;
                case 'h':state += hour_field::parse(&date_char, pack);break;
                case 'm':state += minute_field::parse(&date_char, pack);break;
                case 's':state += second_field::parse(&date_char, pack);break;
                case 'z':state += microsecond_field::parse(&date_char, pack);break;
                default:state += separator_field<1>::parse(&date_char, pack);break;
                }
            }
            return datetime(pack.day, pack.month, pack.year, pack.hour, pack.minute, pack.second, pack.microsecond).data;
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
    datetime::to_string_format(char* out,const char* format, date_format group_format) const
    {
        return datetime_to_string(*this,out, format, group_format);
    }
    
    void
    datetime::add_months(int months)
    {
        datetime_pack pack;
        to_pack(pack);        
        const int new_month = pack.month + months;
        constexpr int february_in_leap = 29;

        if (new_month > 12){
            pack.year = pack.year + new_month / 12;
            pack.month = new_month % 12  == 0 ? 1 : new_month % 12;   
        }
        else{
            pack.month = new_month;
        }
        if (pack.day > days_in_month[pack.month - 1]) {
            pack.day = days_in_month[pack.month - 1];
        }      
        if (is_leap_year(pack.year) && pack.month == 2) {
            pack.day = february_in_leap;  // February in a leap year
        }
        *this = datetime(pack.day, pack.month, pack.year, pack.hour, pack.minute, pack.second, pack.microsecond);
    }

    void
    datetime::add_years(int years)
    {
        datetime_pack pack;
        to_pack(pack);
        pack.year += years;
        constexpr int february_in_leap = 29;
        if (pack.day > days_in_month[pack.month - 1]) {
            pack.day = days_in_month[pack.month - 1];
        }      
        if (is_leap_year(pack.year) && pack.month == 2) {
            pack.day = february_in_leap;  // February in a leap year
        }
        *this = datetime(pack.day, pack.month, pack.year, pack.hour, pack.minute, pack.second, pack.microsecond); 
    }
    
    void
    datetime::to_pack(datetime_pack& pack) const
    {
        epoch_to_datetime_pack(data, pack);
    }

    int
    datetime::day() const
    {
        datetime_pack pack;
        epoch_to_datetime_pack(data, pack);
        return pack.day;
    }
    
    int
    datetime::month() const
    {
        datetime_pack pack;
        epoch_to_datetime_pack(data, pack);
        return pack.month;
    }
    
    int
    datetime::year() const
    {
        datetime_pack pack;
        epoch_to_datetime_pack(data, pack);
        return pack.year;
    }

    int
    datetime::second() const
    {
        datetime_pack pack;
        epoch_to_datetime_pack(data, pack);
        return pack.second;
    }
    
    int
    datetime::minute() const
    {
        datetime_pack pack;
        epoch_to_datetime_pack(data, pack);
        return pack.minute;
    }

    int
    datetime::hour() const
    {
        datetime_pack pack;
        epoch_to_datetime_pack(data, pack);
        return pack.hour;
    }

    int
    datetime::microsecond() const
    {
        datetime_pack pack;
        epoch_to_datetime_pack(data, pack);
        return pack.microsecond;
        
    }
};

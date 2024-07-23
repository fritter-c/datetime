#ifndef GTR_DATETIME_H
#define GTR_DATETIME_H

constexpr auto DATETIME_DEFAULT_FORMAT = "DD/MM/YYYY hh:mm:ss";
constexpr auto DATETIME_INVALID = -9223372036854775807LL - 1LL;

namespace gtr{
    enum class date_format{
        text_date,
        iso_date};
    
    struct datetime_pack{
        int year;
        unsigned char month;
        unsigned char day;
        
        unsigned char hour;
        unsigned char minute;
        unsigned char second;

        unsigned int microsecond;
    };
    
    struct datetime{
        long long data;
     
        inline datetime() : data(0){};
        inline datetime(long long _data) : data(_data){};
        
        datetime(int day, int month, int year, int hour = 0, int minute = 0, int second = 0, int microsecond = 0);
        datetime(const char* date, const char* format = DATETIME_DEFAULT_FORMAT, date_format group_format = date_format::text_date);

        // trivial
        ~datetime() = default;
        datetime(const datetime& other) = default;
        datetime(datetime&& other) noexcept = default;
        datetime& operator=(const datetime& other) = default;
        datetime& operator=(datetime&& other) noexcept = default;

        inline bool
        is_valid(){return data != DATETIME_INVALID;}
        
        bool
        to_string_format(char* out, char* error,const char* format = DATETIME_DEFAULT_FORMAT, date_format group_format = date_format::text_date) const;

        bool
        from_string(const char* date, const char* format = DATETIME_DEFAULT_FORMAT, date_format group_format = date_format::text_date);

        void
        to_pack(datetime_pack&) const;

        inline void
        from_pack(datetime_pack& pack){*this = datetime(pack.day, pack.month, pack.year, pack.hour, pack.minute, pack.second, pack.microsecond);}

        // All operations can be done locally as long as the day does not change otherwise a full POSIX recalc is needed
        inline void
        add_microseconds(long long microseconds)
        {
            long long current_microseconds = 0;
            if (data > 0)
                current_microseconds = static_cast<int>(data % 1000000LL);
            else {
                current_microseconds = static_cast<int>(-data % 1000000LL);
            }
            if (current_microseconds + microseconds > 999999){
                add_seconds(current_microseconds + microseconds / 1000000LL);
            }
         
            data += microseconds % 1000000LL;
        }

        inline void
        add_seconds(int seconds)
        {
            long long sec_per_day = data % (24 * 60 * 60);
            if (sec_per_day < 0)
            {
                sec_per_day += 24 * 60 * 60;
            }
            int current_seconds = (sec_per_day % (60 * 60)) % 60;
            if (current_seconds + seconds > 59){
                add_minutes((current_seconds + seconds) / 60);
            }
            data += (seconds % 60) * 1000000LL;
        }

        inline void
        add_minutes(int minutes)
        {
            long long sec_per_day = data % (24 * 60 * 60);
            if (sec_per_day < 0)
            {
                sec_per_day += 24 * 60 * 60;
            }
            int current_minutes = (sec_per_day % (60 * 60)) / 60;
            if (current_minutes + minutes > 60){
                add_hours((current_minutes + minutes) / 60);
            }
            data += (minutes % 60) * 60LL * 1000000LL;
        }

        inline void
        add_hours(int hours)
        {
            long long sec_per_day = data % (24 * 60 * 60);
            if (sec_per_day < 0)
            {
                sec_per_day += 24 * 60 * 60;
            }
            int current_hour = sec_per_day / (60 * 60);
            if (current_hour + hours > 23){
                add_days((current_hour + hours) / 24);
            }
            data += (hours % 24) * 60LL * 60LL * 1000000LL;
        }

        void
        add_days(int days);

        void
        add_months(int months);

        void
        add_years(int years);
   
        inline bool
        operator==(datetime other){return data == other.data;}

        inline bool
        operator!=(datetime other){return data != other.data;}

        inline bool
        operator>(datetime other){return data > other.data;}

        inline bool
        operator<(datetime other){return data < other.data;}

        inline bool
        operator>=(datetime other){return data >= other.data;}

        inline bool
        operator<=(datetime other){return data <= other.data;}

        inline datetime
        operator+(datetime other){return data + other.data;}

        inline datetime
        operator+=(datetime other) { data += other.data; return *this; }

        inline datetime
        operator-(datetime other){return data - other.data;}

        inline datetime
        operator-=(datetime other) { data -= other.data; return *this; }
    };
};
#endif

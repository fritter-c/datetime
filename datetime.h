#ifndef GTR_DATETIME_H
#define GTR_DATETIME_H
constexpr auto DATETIME_DEFAULT_FORMAT = "DD/MM/YYYY hh:mm:ss";
constexpr auto DATETIME_INVALID = -9223372036854775807LL - 1LL;

namespace gtr{
    enum class date_format{
        text_date,
        iso_date
    };

    struct datetime_pack{
        int year;
        unsigned char month;
        unsigned char day;
        
        unsigned char hour;
        unsigned char minute;
        unsigned char second;

        int microsecond;
    };
    
    struct datetime{
        long long data;
     
        inline constexpr datetime() : data(0){};
        inline constexpr datetime(long long _data) : data(_data){};
        
        datetime(int day, int month, int year, int hour = 0, int minute = 0, int second = 0, int microsecond = 0);
        datetime(const char* date, const char* format = DATETIME_DEFAULT_FORMAT, date_format group_format = date_format::text_date);

        // trivial
        ~datetime() = default;
        datetime(const datetime& other) = default;
        datetime(datetime&& other) noexcept = default;
        datetime& operator=(const datetime& other) = default;
        datetime& operator=(datetime&& other) noexcept = default;

        inline bool
        is_valid() const {return data != DATETIME_INVALID;} 
        
        bool
        to_string_format(char* out, char* error,const char* format = DATETIME_DEFAULT_FORMAT, date_format group_format = date_format::text_date) const;

        bool
        from_string(const char* date, const char* format = DATETIME_DEFAULT_FORMAT, date_format group_format = date_format::text_date);

        void
        to_pack(datetime_pack&) const;

        inline void
        from_pack(datetime_pack& pack){*this = datetime(pack.day, pack.month, pack.year, pack.hour, pack.minute, pack.second, pack.microsecond);}

        inline constexpr void
        add_microseconds(long long microseconds){data += microseconds;}

        inline constexpr void
        add_seconds(long long seconds){data += seconds * 1000000LL;}

        inline constexpr void
        add_minutes(int minutes){data += minutes * 60LL * 1000000LL;}

        inline constexpr void
        add_hours(int hours){data += hours * 60LL * 60LL * 1000000LL;}

        inline constexpr void
        add_days(int days){data += days * 86400 * 1000000LL;}

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
        operator+=(datetime other){data += other.data; return *this; }

        inline datetime
        operator-(datetime other){return data - other.data;}

        inline datetime
        operator-=(datetime other) {data -= other.data; return *this; }
    };  
};
#endif

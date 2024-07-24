#ifndef DATETIME_PARSER
#define DATETIME_PARSER
#include "datetime.h"
#include <cstdlib>
#define __IS_ALPHA(A) ((((A) >= '0') && ((A) <= '9')))
namespace gtr{
   struct year_field{
        inline void operator()(const char** state, datetime_pack& pack)
        {
            char buffer[8];
            int index = 0;
            while(__IS_ALPHA(**state)){
                buffer[index++] = *(*state)++;
            }
            buffer[index] = '\0';
            pack.year = std::atoi(buffer);
        }
    };
    struct day_field{
        inline void operator()(const char** state, datetime_pack& pack)
        {
            char buffer[8];          
            buffer[0] = *(*state)++;
            buffer[1] = *(*state)++;
            buffer[2] = '\0';
            pack.day = std::atoi(buffer);
        }
    };

    struct month_field{
        inline void operator()(const char** state, datetime_pack& pack)
        {
            char buffer[8];          
            buffer[0] = *(*state)++;
            buffer[1] = *(*state)++;
            buffer[2] = '\0';
            pack.month = std::atoi(buffer);
        }
    };

    struct hour_field{
        inline void operator()(const char** state, datetime_pack& pack)
        {
            char buffer[8];          
            buffer[0] = *(*state)++;
            buffer[1] = *(*state)++;
            buffer[2] = '\0';
            pack.hour = std::atoi(buffer);
        }
    };
    struct minute_field{
        inline void operator()(const char** state, datetime_pack& pack)
        {
            char buffer[8];          
            buffer[0] = *(*state)++;
            buffer[1] = *(*state)++;
            buffer[2] = '\0';
            pack.minute = std::atoi(buffer);      
        }
    };
    
    struct second_field{
        inline void operator()(const char** state, datetime_pack& pack)
        {
            char buffer[8];          
            buffer[0] = *(*state)++;
            buffer[1] = *(*state)++;
            buffer[2] = '\0';
            pack.second = std::atoi(buffer);
        }
    };
    
    template<int Digits = 6>struct microsecond_field{
        inline void operator()(const char** state, datetime_pack& pack)
        {
            char buffer[8];
            int i = 0;
            for(i = 0; i < Digits-1; i++){
                buffer[i] = *(*state)++;
            }
            buffer[i] = *(*state)++;
            buffer[Digits] = '\0';
            pack.microsecond = std::atoi(buffer);
        }
    };

    template<int Count = 1> struct separator_field{
        inline void operator()(const char** state, datetime_pack& pack)
        {
            (void)pack;
            (*state)++;
        }
    };
    
    template<class...Args>
    struct perfect_parser{
        static datetime parse_datetime(const char* date)
        {
            datetime_pack pack;
            const char* state = date;
            parse_impl(&state, pack);
            return datetime{pack.day, pack.month, pack.year, pack.hour, pack.minute, pack.second, pack.microsecond};           
        }
    private:
        static void parse_impl(const char** state, datetime_pack& pack)
        {
            (Args{}(state, pack), ...);
        }
    };

}
#endif

#include "datetime.h"
#include <iostream>
#include <chrono>
using gtr::datetime;
using namespace std;
using namespace chrono;
int main(){    
    datetime dt = "03/11/1995 01:30:00";
    const auto now = system_clock::now();
    datetime dt2 = duration_cast<microseconds>(now.time_since_epoch()).count(); 

    char buffer[64];

    //Writes dt in another format
    dt.to_string_format(buffer, "YYYY/MMM/DD");
    cout << "Date I was born -> " << buffer << "\n";

    //Separators are just ignored so 'of' is also written to the buffer
    dt2.to_string_format(buffer, "MMM DD hh:mm:ss:zzz of YYYY");
    cout << "System time now (UTC+00:00) -> " << buffer << "\n"; 
    return 0;
};

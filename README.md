# datetime
Platform agnostic DateTime library written in C++.

This is a datetime library that wraps POSIX time. It does not use any c++ nor c headers, everything is self-contained in .h and .cpp file

This library does not account for leap seconds and datetime ALWAYS holds a timestamp UTC+00:00. 

When manipulating time (e.g. adding months, years) sometimes impossible dates can be reached for example if you are on January 31 and add one month, the date would be "February 31" which is not a real date. In these cases
the library always correct down the days and the add month operation would result in February 28 in a non-leap year or February 29 in a leap-year. So, in a nutshell, when adding or subtracting years or months, be aware that
datetime can correct down the day if it extrapolates the limit to fit the new month.

Unlike C-like time functions, this class operates with a precision of microseconds so the maximum years it can holds are +/- 290,000

The string parsers CURRENTLY work ONLY with positive years (1 - 290000)

The string parsers are NOT SAFE, so external inputs are not verified for consistency. Always use in-code format strings and date input strings, verify your datetime source (e.g. your .csv) or code a safe version yourself

Do not use datetime string functions(atoi, strcpy...) for anything else. They're adapted to be used within the datetime library and do not serve as a replacement for CRT default ones.

Warning:
  This library is still under developement and testing. Be mindful when using it in critical systems

# perfect_parser

  A utility to parse string datetime if you know the format at compile time, this could be useful to parse large files faster 

  - Example "YYYY-MM-DD hh:mm:ss.zzzzzz"

        datetime other = perfect_parser<year_field,
                                        separator_field<1>,
                                        month_field<>,
                                        separator_field<1>,
                                        day_field,
                                        separator_field<1>,
                                        hour_field,
                                        separator_field<1>,
                                        minute_field,
                                        separator_field<1>,
                                        second_field,
                                        separator_field<1>,
                                        microsecond_field<>::parse_datetime(datetime_string);
  

This will create a function that executes the correct parsing instructions in the correct order(year, separator, month...) so a swicth statement is no longer necessary resulting in less branches and state verification.

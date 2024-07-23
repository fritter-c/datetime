# datetime
Platform agnostic DateTime library written in C++.

This is a datetime library that wraps POSIX time. It does not use any c++ nor c headers, everything is self-contained in .h and .cpp file

This library does not account for leap seconds and datetime ALWAYS holds a timestamp UTC+00:00. 

Unlike C-like time functions, this class operates with a precision of microseconds so the maximum years it can holds are +/- 290,000

The string parsers are NOT SAFE, so external inputs are not verified for consistency. Always use in-code format strings and date input strings, verify your datetime source (e.g. your .csv) or code a safe version yourself

Warning:
  This library is still under developement and testing. Be mindful when using it in critical systems

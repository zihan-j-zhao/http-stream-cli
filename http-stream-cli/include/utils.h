#pragma once

#include <chrono>
#include <string>

#include <date/date.h>

namespace httpstream
{
    namespace utils
    {
        class DateUtils
        {
        public:
            static inline long long SecFromDate(const std::string& _date, const std::string& _format)
            {
                std::istringstream in{ _date };
                date::sys_seconds tp;
                in >> date::parse(_format, tp);
                if (in.fail())
                    throw std::invalid_argument("Incorrect date format");
                return tp.time_since_epoch().count();
            }

            static inline date::year_month_day YmdFromSec(long long sec)
            {
                std::chrono::seconds dur(sec);
                std::chrono::time_point<std::chrono::system_clock> tp(dur);
                return { floor<date::days>(tp) };
            }
        };
    }
}

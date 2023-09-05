#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <algorithm>

#include <date/date.h>

namespace httpstream
{
    namespace utils
    {
        template<typename _T>
        bool present_in(const _T& target, const std::vector<_T>& pool)
        {
            return std::find(pool.begin(), pool.end(), target) != pool.end();
        }

        template<typename _T>
        class Range
        {
        private:
            const _T _a, _b;

        public:
            Range(_T a, _T b) : _a(a), _b(b) { }

            _T Lower() const
            {
                return _a <= _b ? _a : _b;
            }

            _T Upper() const
            {
                return _a <= _b ? _b : _a;
            }
        };

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

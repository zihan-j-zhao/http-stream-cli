#pragma once

#include <random>
#include <string>

#include <date/date.h>
#include <nlohmann/json.hpp>

#include "utils.h"
#include "source/source.h"

namespace httpstream
{
    namespace source
    {
        class RandInt : public Source
        {
        protected:
            const utils::Range<int> _range;

        public:
            RandInt(const utils::Range<int>& range) : _range(range) { }

            nlohmann::json Load() override
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution distrib(_range.Lower(), _range.Upper());
                return distrib(gen);
            }
        };

        class RandDouble : public Source
        {
        protected:
            const utils::Range<double> _range;

        public:
            RandDouble(const utils::Range<double>& range) : _range(range) { }

            nlohmann::json Load() override
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<double> distrib(_range.Lower(), m_range.Upper());
                return distrib(gen);
            }
        };

        class RandDate : public Source
        {
        protected:
            const std::string _format;
            long long _lower, _upper;

        public:
            RandDate(const std::string& begin, const std::string& end, const std::string& format)
                : _lower(utils::DateUtils::SecFromDate(begin, format)), 
                  _upper(utils::DateUtils::SecFromDate(end, format)), 
                  _format(format) { }

            nlohmann::json Load() override
            {
                return date::format(_format, utils::DateUtils::YmdFromSec(next_long_long()));
            }

        private:
            inline long long next_long_long()
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<long long> distrib(m_lower, m_upper);
                return distrib(gen);
            }
        };
    }
}

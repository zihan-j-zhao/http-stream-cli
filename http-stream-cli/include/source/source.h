#pragma once

#include <string>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace httpstream
{
    namespace source
    {
        class Source
        {
        public:
            virtual ~Source() = default;
            virtual nlohmann::json Load() = 0;

            class EndOfRead : public std::runtime_error
            {
            public:
                EndOfRead(const std::string& what = "") : std::runtime_error(what) {}
            };

            class InferredEndOfRead : public EndOfRead
            {
            public:
                InferredEndOfRead(const std::string& what = "") : EndOfRead(what) {}
            };

            class MismatchedType : public std::runtime_error
            {
            public:
                MismatchedType(const std::string& what = "") : std::runtime_error(what) {}
            };
        };

        class DefaultSource : public Source
        {
        private:
            const nlohmann::json _default;

        public:
            DefaultSource(const nlohmann::json& default) : _default(default) { }

            nlohmann::json Load() override
            {
                return _default;
            }
        };
    }
}

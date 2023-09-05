#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <functional>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "utils.h"

namespace httpstream
{
    inline namespace parser
    {
        static const inline std::vector<std::string> SRC_TYPES = { "xlsx", "rand-int", "rand-double", "rand-date" };
        static const inline std::vector<std::string> VAL_TYPES = { "int", "bool", "double", "string" };
        static const inline std::vector<std::string> ONE_TYPES = { "int", "bool", "double", "string", "map" };
        static const inline std::vector<std::string> MUL_TYPES = { "array-int", "array-bool", "array-double", "arary-string", "array-map" };

        std::string stringify(const std::vector<std::string>& vec)
        {
            std::stringstream ss;
            for (auto it = vec.begin(); it != vec.end(); ++it)
            {
                if (it != vec.begin()) ss << ", ";
                ss << *it;
            }
            return ss.str();
        }

        template<typename... Args>
        bool assert_(bool expr, bool ignore, std::string fmt, Args &&...args)
        {
            if (!expr && !ignore) spdlog::error(fmt, args...);
            return expr;
        }

        bool exist(const nlohmann::json& json, const std::string& key, bool ignore = false)
        {
            return assert_(json.contains(key), ignore, "Missing {} field", key);
        }

        bool ex_and_str(const nlohmann::json& json, const std::string& key, bool ignore = false)
        {
            return exist(json, key, ignore) && assert_(json[key].is_string(), ignore, "Wrong type for {} field, expected string", key);
        }

        bool ex_and_obj(const nlohmann::json& json, const std::string& key, bool ignore = false)
        {
            return exist(json, key, ignore) && assert_(json[key].is_object(), ignore, "Wrong type for {} field, expected object", key);
        }

        bool ex_and_int(const nlohmann::json& json, const std::string& key, bool ignore = false)
        {
            return exist(json, key, ignore) && assert_(json[key].is_number_integer(), ignore, "Wrong type for {} field, expected integer", key);
        }

        bool ex_and_float(const nlohmann::json& json, const std::string& key, bool ignore = false)
        {
            return exist(json, key, ignore) && assert_(json[key].is_number_float(), ignore, "Wrong type for {} field, expected float", key);
        }

        bool ex_and_uint(const nlohmann::json& json, const std::string& key, bool ignore = false)
        {
            return ex_and_int(json, key, ignore) && assert_(json[key].is_number_unsigned(), ignore, "Wrong type for {} field, expected positive integer", key);
        }

        bool check_source(const nlohmann::json& obj)
        {
            spdlog::debug("Validating the below json source:\n {}", obj.dump(2));

            if (!ex_and_str(obj, "type")) return false;

            std::string type = obj["type"].get<std::string>();
            if (!utils::present_in(type, SRC_TYPES))
            {
                spdlog::error("Unsupported source type: {} (expected one of {})", type, stringify(SRC_TYPES));
                return false;
            }

            if (type == "xlsx")
            {
                if (!ex_and_str(obj, "path")) return false;
                if (!ex_and_str(obj, "sheet")) return false;
                if (!ex_and_str(obj, "column")) return false;
                if (!ex_and_uint(obj, "begin")) return false;
                if (!ex_and_uint(obj, "end")) return false;
                if (!ex_and_str(obj, "vtype")) return false;

                std::string vtype = obj["vtype"].get<std::string>();
                if (!utils::present_in(vtype, VAL_TYPES))
                {
                    spdlog::error("Unsupported source vtype: {} (expected one of {})", vtype, stringify(VAL_TYPES));
                    return false;
                }
            }
            else if (type == "rand-int")
            {
                if (!ex_and_int(obj, "begin")) return false;
                if (!ex_and_int(obj, "end")) return false;
            }
            else if (type == "rand-double")
            {
                if (!ex_and_float(obj, "begin")) return false;
                if (!ex_and_float(obj, "end")) return false;
            }
            else
            {
                if (!ex_and_str(obj, "begin")) return false;
                if (!ex_and_str(obj, "end")) return false;
                if (!ex_and_str(obj, "dateFormat")) return false;

                std::string format = obj["dateFormat"].get<std::string>();
                long long b_epoch = utils::DateUtils::SecFromDate(obj["begin"].get<std::string>(), format);
                long long e_epoch = utils::DateUtils::SecFromDate(obj["end"].get<std::string>(), format);
                assert_(b_epoch <= e_epoch, false, "Invalid date range");
            }

            return true;
        }

        bool check_object(const nlohmann::json& obj)
        {
            spdlog::debug("Validating the below json object:\n {}", obj.dump(2));

            if (!ex_and_str(obj, "+type")) return false;

            std::string type = obj["+type"].get<std::string>();
            if (type == "map" || type == "array-map")
            {
                if (ex_and_obj(obj, "+source", true))
                {
                    spdlog::warn("Ignoring +source field in case of {}", type);
                }
            }
            else if (utils::present_in<std::string>(type, ONE_TYPES) || utils::present_in<std::string>(type, MUL_TYPES))
            {
                if (!ex_and_obj(obj, "+source")) return false;
                if (!check_source(obj["+source"])) return false;

                if (utils::present_in<std::string>(type, ONE_TYPES))
                {
                    if (ex_and_uint(obj, "+size", true) && obj["+size"].get<int>() != 1)
                    {
                        spdlog::warn("Ignoring +size field in case of type {} (default to 1)", type);
                    }
                }
                else
                {
                    if (!exist(obj, "+size", true))
                    {
                        spdlog::warn("No +size field for array type (default to 1)");
                    }
                    else if (!ex_and_uint(obj, "+size"))
                    {
                        return false;
                    }
                }
            }
            else
            {
                spdlog::error("Unsupported entry type: {} (expected one of {}, {})", type, stringify(ONE_TYPES), stringify(MUL_TYPES));
                return false;
            }

            for (auto& el : obj.items())
                if (!utils::present_in<std::string>(el.key(), { "+type", "+size", "+source" })
                    && el.value().is_object() && !check_object(el.value()))
                    return false;
            return true;
        }
    }
}

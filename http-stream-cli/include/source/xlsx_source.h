#pragma once

#include <map>
#include <string>
#include <vector>
#include <stdexcept>

#include <OpenXLSX.hpp>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "utils.h"
#include "source/source.h"

namespace httpstream
{
    namespace source
    {
        class XLSXSource : public Source
        {
        private:
            using RealValueType = OpenXLSX::XLValueType;
            static const inline std::map<std::string, std::vector<RealValueType>> k_type_map = {
                { "int", { RealValueType::Integer, RealValueType::Empty } },
                { "bool", { RealValueType::Boolean, RealValueType::Empty } },
                { "double", { RealValueType::Float, RealValueType::Empty } },
                { "string", { RealValueType::String, RealValueType::Empty } }
            };
            static constexpr uint64_t k_infer_threshold = 10;  // default

            const std::string _path;
            const std::string _sheet;
            const std::string _column;
            const std::string _type;
            const utils::Range<uint32_t> _range;

            OpenXLSX::XLDocument _doc;
            OpenXLSX::XLWorksheet _sheet;
            uint32_t _cur;

        public:
            XLSXSource(const std::string& path, const std::string& sheet, const std::string& column, const std::string& type, utils::Range<uint32_t>& range)
                : _path(path), _sheet(sheet), _column(column), _type(type), _range(range), _cur(range.Lower())
            {
                _doc.open(_path);
                if (!_doc.isOpen())
                {
                    spdlog::error("File not existed: {}", _path);
                    throw std::invalid_argument("Invalid file path");
                }
                if (!_doc.workbook().worksheetExists(_sheet))
                {
                    spdlog::error("Sheet not existed: {} ({})", _sheet, _path);
                    throw std::invalid_argument("Invalid sheet name");
                }

                _sheet = _doc.workbook().worksheet(_sheet);
                uint16_t col = OpenXLSX::XLCellReference::columnAsNumber(_column);
                if (col < 1 || _sheet.columnCount() < col)
                {
                    std::string last = OpenXLSX::XLCellReference::columnAsString(_sheet.columnCount());
                    spdlog::error("Column {} (#{}) lies outside the table (expected #{}~#{}) ({})",
                        _column, col, 1, last, _path);
                    throw std::invalid_argument("Column outside the table");
                }

                if (_range.Lower() < 1 || _sheet.rowCount() < _range.Lower())
                {
                    spdlog::error("Begin row #{} lies outside the table (expected #{}~#{}) ({})",
                        _range.Lower(), 1, _sheet.rowCount(), _path);
                    throw std::invalid_argument("Row outisde the table");
                }
            }

            ~XLSXSource()
            {
                _doc.close();
            }

        private:
            inline bool is_cell_empty(std::string& cell) const
            {
                return _sheet.cell(cell).value().type() == RealValueType::Empty;
            }

            inline std::string cell(int32_t delta = 0) const
            {
                return _column + std::to_string(_cur + delta);
            }

            void assert_within_boundary() const
            {
                if (_cur > _range.Upper())
                {
                    spdlog::info("All rows (#{}~#{}) have been read ({})", _range.Lower(), _range.Upper(), _path);
                    throw EndOfRead();
                }
            }

            void assert_matched_type(std::string& cell) const
            {
                auto real_type = _sheet.cell(cell).value().type();
                std::string real_type_str = _sheet.cell(cell).value().typeAsString();

                if (!is_type_matched(real_type))
                {
                    spdlog::error("Expected type {}, but got type {} ({})", real_type_str, _type, _path);
                    throw MismatchedType();
                }
            }

            inline bool is_type_matched(RealValueType type) const
            {
                auto& v = k_type_map.find(_type)->second;
                return utils::present_in(_type, v);
            }

            void ensure_safety_read()
            {
                uint64_t i = 0;
                do
                {
                    assert_within_boundary();
                    assert_matched_type(cell());
                    if (!is_cell_empty(cell())) return;
                    spdlog::debug("Row {} skipped upon an empty cell ({})", _cur, _path);
                    ++m_cur;
                } while (++i < k_infer_threshold);

                spdlog::info("Due to {} consecutive empty cells, it was inferred that all rows have been read ({})",
                    k_infer_threshold, _path);
                throw InferredEndOfRead();
            }

            template<class _Type>
            _Type read_next()
            {
                ensure_safety_read();
                return _sheet.cell(_column + std::to_string(_cur++)).value().get<_Type>();
            }
        };
    }
}

#pragma once

#include "base.hpp"

#include <cstdint>
#include <cstddef>
#include <unordered_set>
#include <vector>

NAMESPACE_SOUP
{
	struct HidReportDescriptor
	{
		uint16_t usage_page = 0;
		uint16_t usage = 0;
		uint16_t input_report_byte_length = 0; // including report id
		uint16_t output_report_byte_length = 0; // including report id
		uint16_t feature_report_byte_length = 0; // including report id
		std::unordered_set<uint8_t> report_ids{};

		struct ReportField
		{
			bool is_variable;
			uint16_t usage_page;
			uint16_t usage_min;
			uint32_t size;
			uint32_t count;
		};
		std::vector<ReportField> input_report_fields{};

		[[nodiscard]] static HidReportDescriptor parse(const void* _rawdesc, size_t size);

		[[nodiscard]] std::vector<uint32_t> parseInputReport(const void* report, size_t size) const;
	};
}

#include "HidReportDescriptor.hpp"

//#include <iostream>

#include "BitReader.hpp"
#include "MemoryRefReader.hpp"

NAMESPACE_SOUP
{
	static uint32_t get_hid_report_bytes(const uint8_t* rawdesc, size_t size, uint32_t num_bytes, uint32_t pos)
	{
		if (pos + num_bytes < size)
		{
			if (num_bytes == 1)
			{
				return rawdesc[pos + 1];
			}
			else if (num_bytes == 2)
			{
				return (rawdesc[pos + 2] * 256 + rawdesc[pos + 1]);
			}
			else if (num_bytes == 4)
			{
				return (
					rawdesc[pos + 4] * 0x01000000 +
					rawdesc[pos + 3] * 0x00010000 +
					rawdesc[pos + 2] * 0x00000100 +
					rawdesc[pos + 1] * 0x00000001
					);
			}
		}
		return 0;
	}

	/*
	 * Gets the size of the HID item at the given position
	 * Returns 1 if successful, 0 if an invalid key
	 * Sets data_len and key_size when successful
	 */
	static bool get_hid_item_size(const uint8_t* rawdesc, size_t size, uint32_t pos, uint32_t* data_len, uint32_t* key_size)
	{
		const auto key = rawdesc[pos];

		/*
		 * This is a Long Item. The next byte contains the
		 * length of the data section (value) for this key.
		 * See the HID specification, version 1.11, section
		 * 6.2.2.3, titled "Long Items."
		 */
		if ((key & 0xf0) == 0xf0)
		{
			if (pos + 1 < size)
			{
				*data_len = rawdesc[pos + 1];
				*key_size = 3;
				return true;
			}
			*data_len = 0; /* malformed report */
			*key_size = 0;
		}

		/*
		 * This is a Short Item. The bottom two bits of the
		 * key contain the size code for the data section
		 * (value) for this key. Refer to the HID
		 * specification, version 1.11, section 6.2.2.2,
		 * titled "Short Items."
		 */
		const auto size_code = key & 0x3;
		switch (size_code)
		{
		case 0:
		case 1:
		case 2:
			*data_len = size_code;
			*key_size = 1;
			return true;
		case 3:
			*data_len = 4;
			*key_size = 1;
			return true;
		default:
			/* Can't ever happen since size_code is & 0x3 */
			SOUP_UNREACHABLE;
		};

		/* malformed report */
		return false;
	}

	HidReportDescriptor HidReportDescriptor::parse(const void* _rawdesc, size_t size)
	{
		const auto rawdesc = reinterpret_cast<const uint8_t*>(_rawdesc);

		HidReportDescriptor parsed;
		uint16_t usage_page = 0;
		uint32_t input_report_bit_length = 0;
		uint32_t output_report_bit_length = 0;
		uint32_t feature_report_bit_length = 0;

		std::vector<uint16_t> usage_ids{};
		uint32_t report_size = 0;
		uint32_t report_count = 0;
		uint32_t usage_min = 0;
		uint32_t logical_max = 0;

		for (uint32_t pos = 0; pos < size; )
		{
			const auto key = rawdesc[pos];
			const auto key_cmd = key & 0xfc;

			uint32_t data_len, key_size;
			if (!get_hid_item_size(rawdesc, size, pos, &data_len, &key_size))
			{
				break;
			}

			//std::cout << std::hex << key_cmd << ": " << get_hid_report_bytes(rawdesc, size, data_len, pos) << std::endl;

			switch (key_cmd)
			{
			case 0x04: /* Usage Page 6.2.2.7 (Global) */
				usage_page = get_hid_report_bytes(rawdesc, size, data_len, pos);
				if (parsed.usage_page == 0)
				{
					parsed.usage_page = usage_page;
				}
				break;

			case 0x08: /* Usage 6.2.2.8 (Local) */
				{
					uint16_t usage_id;
					if (data_len == 4) /* Usages 5.5 / Usage Page 6.2.2.7 */
					{
						usage_page = get_hid_report_bytes(rawdesc, size, 2, pos + 2);
						if (parsed.usage_page == 0)
						{
							parsed.usage_page = usage_page;
						}
						usage_id = get_hid_report_bytes(rawdesc, size, 2, pos);
					}
					else
					{
						usage_id = get_hid_report_bytes(rawdesc, size, data_len, pos);
					}
					if (parsed.usage == 0)
					{
						parsed.usage = usage_id;
					}
					else
					{
						usage_ids.emplace_back(usage_id);
					}
				}
				break;

			case 0x74: // Report Size
				report_size = get_hid_report_bytes(rawdesc, size, data_len, pos);
				break;

			case 0x94: // Report Count
				report_count = get_hid_report_bytes(rawdesc, size, data_len, pos);
				break;

			case 0x80: // Input
				{
					input_report_bit_length += (report_size * report_count);

					const auto flags = get_hid_report_bytes(rawdesc, size, data_len, pos);
					const bool is_variable = ((flags >> 1) & 1) != 0;
					const bool has_null = ((flags >> 6) & 1) != 0;
					if (!is_variable)
					{
						usage_ids.clear();
					}
					parsed.input_report_fields.emplace_back(ReportField{
						report_size,
						report_count,
						logical_max + has_null,
						is_variable,
						usage_page,
						std::move(usage_ids)
					});
					usage_ids.clear();
				}
				break;

			case 0x90: // Output
				usage_ids.clear();
				output_report_bit_length += (report_size * report_count);
				break;

			case 0xB0: // Feature
				usage_ids.clear();
				feature_report_bit_length += (report_size * report_count);
				break;

			case 0x84: // Report ID
				parsed.report_ids.emplace(static_cast<uint8_t>(get_hid_report_bytes(rawdesc, size, data_len, pos)));
				break;

			case 0x18: // Usage Minimum
				usage_min = get_hid_report_bytes(rawdesc, size, data_len, pos);
				break;

			case 0x28: // Usage Maximum
				{
					uint32_t usage_max = get_hid_report_bytes(rawdesc, size, data_len, pos);
					for (uint32_t usage = usage_min; usage != (usage_max + 1); ++usage)
					{
						usage_ids.emplace_back(usage);
					}
				}
				break;

			case 0x14: // Logical Minimum
				break;

			case 0x24: // Logical Maximum
				logical_max = get_hid_report_bytes(rawdesc, size, data_len, pos);
				break;
			}

			pos += data_len + key_size;
		}

		parsed.input_report_byte_length = ((input_report_bit_length + 7) >> 3) + 1; // plus report id
		parsed.output_report_byte_length = ((output_report_bit_length + 7) >> 3) + 1; // plus report id
		parsed.feature_report_byte_length = ((feature_report_bit_length + 7) >> 3) + 1; // plus report id

		return parsed;
	}

	HidParsedReport HidReportDescriptor::parseInputReport(const void* report, size_t size) const
	{
		HidParsedReport parsed;

		MemoryRefReader mr(report, size);
		if (!report_ids.empty())
		{
			mr.skip(1); // Ignore report id
		}
		BitReader br(&mr);
		for (const auto& f : input_report_fields)
		{
			if (f.is_variable)
			{
				if (f.size == 1)
				{
					auto pUsageId = f.usage_ids.cbegin();
					for (uint32_t i = 0; i != f.count; ++i)
					{
						bool on = false;
						SOUP_UNUSED(br.b(on));
						if (pUsageId != f.usage_ids.end())
						{
							if (on)
							{
								parsed.active_selectors.emplace(HidUsage(f.usage_page, *pUsageId));
							}
							++pUsageId;
						}
					}
					continue;
				}
				else if (f.size <= 8 && f.logical_max != 0)
				{
					auto pUsageId = f.usage_ids.cbegin();
					for (uint32_t i = 0; i != f.count; ++i)
					{
						uint8_t value = 0;
						SOUP_UNUSED(br.u8(f.size, value));
						if (pUsageId != f.usage_ids.end())
						{
							parsed.dynamic_values.emplace(HidUsage(f.usage_page, *pUsageId), static_cast<float>(value) / static_cast<uint8_t>(f.logical_max));
							++pUsageId;
						}
					}
					continue;
				}
				else if (f.size == 16 && f.logical_max != 0)
				{
					auto pUsageId = f.usage_ids.cbegin();
					for (uint32_t i = 0; i != f.count; ++i)
					{
						uint8_t hi = 0;
						uint8_t lo = 0;
						SOUP_UNUSED(br.u8(8, lo));
						SOUP_UNUSED(br.u8(8, hi));
						uint16_t value = ((static_cast<uint16_t>(hi) << 8) | lo);
						if (pUsageId != f.usage_ids.end())
						{
							parsed.dynamic_values.emplace(HidUsage(f.usage_page, *pUsageId), static_cast<float>(value) / static_cast<uint16_t>(f.logical_max));
							++pUsageId;
						}
					}
					continue;
				}
			}
			else // array
			{
				if (f.size == 8)
				{
					for (uint32_t i = 0; i != f.count; ++i)
					{
						uint8_t usage = 0;
						SOUP_UNUSED(br.u8(8, usage));
						if (usage != 0)
						{
							parsed.active_selectors.emplace(HidUsage(f.usage_page, usage));
						}
					}
					continue;
				}
			}

			// Unhandled field; skip it.
			for (uint32_t i = 0; i != f.count * f.size; ++i)
			{
				bool dummy;
				SOUP_UNUSED(br.b(dummy));
			}
		}

		return parsed;
	}
}

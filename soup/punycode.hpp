#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "base.hpp"

NAMESPACE_SOUP
{
	struct punycode
	{
		[[nodiscard]] static std::string encode(const std::u32string& in);
		[[nodiscard]] static std::u32string decode(const std::string& in);

		[[nodiscard]] static std::string encodeDeltas(const std::vector<uint32_t>& deltas);
		[[nodiscard]] static std::vector<uint32_t> decodeDeltas(const char* data);
	};
}

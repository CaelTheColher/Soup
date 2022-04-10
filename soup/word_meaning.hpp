#pragma once

#include "packet.hpp"
#include <string>
#include <vector>
#include "word_type.hpp"

namespace soup
{
	SOUP_PACKET(word_meaning)
	{
		word_type type;
		std::string meaning;
		std::vector<std::string> context;
		std::vector<std::string> example;

		SOUP_PACKET_IO(s)
		{
			if (s.isRead())
			{
				uint8_t this_is_why_you_dont_use_enums;
				if (!s.u8(this_is_why_you_dont_use_enums))
				{
					return false;
				}
				type = (word_type)this_is_why_you_dont_use_enums;
			}
			else if (s.isWrite())
			{
				uint8_t this_is_why_you_dont_use_enums = type;
				if (!s.u8(this_is_why_you_dont_use_enums))
				{
					return false;
				}
			}
			return s.str_nt(meaning)
				&& s.vec_str_nt_u64_dyn(context)
				&& s.vec_str_nt_u64_dyn(example)
				;
		}
	};
}

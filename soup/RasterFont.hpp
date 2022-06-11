#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "fwd.hpp"

namespace soup
{
	struct RasterFont
	{
		struct Glyph
		{
			uint8_t width;
			uint8_t height;
			std::vector<bool> pixels;
			int8_t y_offset = 0;

			Glyph(uint8_t width, uint8_t height, std::vector<bool>&& pixels, int8_t y_offset = 0)
				: width(width), height(height), pixels(std::move(pixels)), y_offset(y_offset)
			{
			}

			[[nodiscard]] Canvas getCanvas() const;
		};

		uint8_t baseline_glyph_height;
		std::unordered_map<uint32_t, Glyph> glyphs;

		[[nodiscard]] static RasterFont simple5(); // 5 pixels tall, not ideal for lowercase characters
		[[nodiscard]] static RasterFont simple8(); // 8 pixels tall

		[[nodiscard]] const Glyph& get(uint32_t c) const;
		[[nodiscard]] uint32_t getFallback() const;

		[[nodiscard]] size_t measureWidth(const std::string& text) const;
		[[nodiscard]] size_t measureWidth(const std::u32string& text) const;
	};
}

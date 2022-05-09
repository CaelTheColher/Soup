#include "RenderTarget.hpp"

#include "RasterFont.hpp"
#include "unicode.hpp"

namespace soup
{
	void RenderTarget::fill(Rgb colour)
	{
		drawRect(0, 0, width, height, colour);
	}

	void RenderTarget::drawText(size_t x, size_t y, const std::string& text, const RasterFont& font, Rgb colour)
	{
		drawText(x, y, unicode::utf8_to_utf32(text), font, colour);
	}

	void RenderTarget::drawText(size_t x, size_t y, const std::u32string& text, const RasterFont& font, Rgb colour)
	{
		for (const auto& c : text)
		{
			const auto& g = font.get(c);
			size_t i = 0;
			for (size_t glyph_y = 0; glyph_y != g.height; ++glyph_y)
			{
				for (size_t glyph_x = 0; glyph_x != g.width; ++glyph_x)
				{
					if (g.pixels.at(i))
					{
						drawRect(x + glyph_x, y + glyph_y + g.y_offset, 1, 1, colour);
					}
					++i;
				}
			}
			x += (g.width + 1);
		}
	}
}

#include "AnalogueKeyboard.hpp"

#include "BufferRefReader.hpp"
#if SOUP_WINDOWS
#include "Window.hpp"
#endif

namespace soup
{
	[[nodiscard]] static const char* checkDeviceName(const hwHid& hid)
	{
		// Wooting, https://github.com/WootingKb/wooting-analog-sdk/blob/develop/wooting-analog-plugin/src/lib.rs
		if (hid.usage_page == 0xFF54)
		{
			if (hid.vendor_id == 0x31E3)
			{
				// Last nibble is for gamepad mode: 0 = Xbox, 1 = Classic, 2 = None
				if ((hid.product_id & 0xFFF0) == 0x1100)
				{
					return "Wooting One";
				}
				if ((hid.product_id & 0xFFF0) == 0x1200)
				{
					return "Wooting Two";
				}
				if ((hid.product_id & 0xFFF0) == 0x1210)
				{
					return "Wooting Lekker";
				}
				if ((hid.product_id & 0xFFF0) == 0x1220)
				{
					return "Wooting Two HE";
				}
				if ((hid.product_id & 0xFFF0) == 0x1230)
				{
					return "Wooting Two HE ARM";
				}
				if ((hid.product_id & 0xFFF0) == 0x1310)
				{
					return "Wooting 60 HE ARM";
				}
			}
			else if (hid.vendor_id == 0x03EB)
			{
				if (hid.product_id == 0xFF01)
				{
					return "Wooting One (Old Firmware)";
				}
				if (hid.product_id == 0xFF02)
				{
					return "Wooting Two (Old Firmware)";
				}
			}
		}

		return nullptr;
	}

	std::vector<AnalogueKeyboard> AnalogueKeyboard::getAll(bool include_no_permission)
	{
		std::vector<AnalogueKeyboard> res{};

		for (auto& hid : hwHid::getAll())
		{
			if (include_no_permission || hid.havePermission())
			{
				if (auto name = checkDeviceName(hid))
				{
					res.emplace_back(AnalogueKeyboard{ name, std::move(hid) });
				}
			}
		}

		return res;
	}

	using ActiveKey = AnalogueKeyboard::ActiveKey;

	std::vector<ActiveKey> AnalogueKeyboard::getActiveKeys()
	{
		std::vector<ActiveKey> keys{};

		auto report = hid.pollReport();
		SOUP_IF_UNLIKELY (report.empty())
		{
			disconnected = true;
		}
		else
		{
			BufferRefReader r(report, false);
			uint16_t scancode;
			uint8_t value;
			while (r.hasMore()
				&& r.u16(scancode)
				&& scancode != 0 // report is always same size, but the after active keys is just 0 bytes
				&& r.u8(value)
				)
			{
				// some keys seem to be getting reported multiple times, so just use last reported value
				for (auto& key : keys)
				{
					if (key.scancode == scancode)
					{
						key.value = value;
						goto _no_emplace;
					}
				}
				keys.emplace_back(ActiveKey{
					scancode,
					value
				});
			_no_emplace:;
			}
		}

		return keys;
	}

#if SOUP_WINDOWS
	int AnalogueKeyboard::ActiveKey::getVkTranslated() const noexcept
	{
		if (int vk = getVkPrecheck())
		{
			return vk;
		}
		const auto layout = GetKeyboardLayout(Window::getFocused().getOwnerTid());
		return MapVirtualKeyExA(getPs2Scancode(), MAPVK_VSC_TO_VK_EX, layout);
	}
#endif
}

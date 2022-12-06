#pragma once

#include "base.hpp"

#if SOUP_WINDOWS
#pragma comment(lib, "winmm.lib")

#include "fwd.hpp"

#include "math.hpp"
#define HZ_TO_ANGVEL(x) ((x) * 2 * M_PI)

#include <string>
#include <vector>

namespace soup
{
	// Return the amplitude (-1.0 to +1.0) at a given point in time (audPlayback::getTime)
	using audGetAmplitude = double(*)(audPlayback&);

	class audDevice
	{
	public:
		int i;
	private:
		std::wstring name;
	public:
		uint16_t max_channels;

		audDevice(int i, std::wstring&& name, uint16_t max_channels)
			: i(i), name(std::move(name)), max_channels(max_channels)
		{
		}

		[[nodiscard]] static audDevice get(int i);
		[[nodiscard]] static audDevice getDefault();
		[[nodiscard]] static std::vector<audDevice> getAll();

		[[nodiscard]] std::string getName() const;
		[[nodiscard]] UniquePtr<audPlayback> open() const;
		UniquePtr<audPlayback> open(audGetAmplitude src, void* user_data = nullptr) const;
	};
}

#endif
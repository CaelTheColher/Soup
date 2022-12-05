#pragma once

#include <chrono>
#include <ctime>

namespace soup
{
	struct Datetime
	{
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
	};

	struct time
	{
		[[nodiscard]] static std::time_t unixSeconds() noexcept
		{
			return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		}

		[[nodiscard]] static std::time_t millis() noexcept
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
		}

		[[nodiscard]] static std::time_t millisSince(std::time_t since) noexcept
		{
			return millis() - since;
		}

		[[nodiscard]] static std::time_t nanos() noexcept
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
		}

		[[nodiscard]] static std::time_t toUnix(const Datetime& dt);
		[[nodiscard]] static std::time_t toUnix(int year, int month, int day, int hour, int minute, int second);
	};
}

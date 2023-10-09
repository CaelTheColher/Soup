#pragma once

#include "fwd.hpp"

#include <cstdint>
#include <string>
#include <utility>

#include "Callback.hpp"
#include "Capture.hpp"

namespace soup
{
	constexpr uint8_t WORKER_TYPE_UNSPECIFIED = 0;
	constexpr uint8_t WORKER_TYPE_SOCKET = 1;
	constexpr uint8_t WORKER_TYPE_TASK = 2;
	constexpr uint8_t WORKER_TYPE_USER = 3;

	struct Worker
	{
		enum HoldupType : uint8_t
		{
			NONE,
			SOCKET,
			IDLE, // call holdup callback whenever the scheduler has some idle time
			PROMISE_VOID,
		};

		enum SchedulingDisposition : int
		{
			HIGH_FRQUENCY = 1 << 1,
			NEUTRAL = 1 << 0,
			LOW_FREQUENCY = 0,
		};

		uint8_t type;
		uint8_t recursions = 0;
		HoldupType holdup_type = NONE;
		Callback<void(Worker&)> holdup_callback;
		void* holdup_data;

		Worker(uint8_t type) noexcept
			: type(type)
		{
		}

		virtual ~Worker() = default;

		Worker& operator=(Worker&& b) noexcept = default;

		void fireHoldupCallback()
		{
			recursions = 0;
			holdup_callback(*this);
		}

		void awaitPromiseCompletion(Promise<void>* p, void(*f)(Worker&, Capture&&), Capture&& cap = {});

		[[nodiscard]] bool isWorkDone() const noexcept { return holdup_type == NONE; }
		void setWorkDone() noexcept { holdup_type = NONE; }

		void disallowRecursion() noexcept { recursions = 19; }
		[[nodiscard]] bool canRecurse() noexcept { return ++recursions != 20; }

		[[nodiscard]] std::string toString() const;
	};
}

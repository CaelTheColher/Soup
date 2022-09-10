#pragma once

#include "fwd.hpp"

#include <cstdint>
#include <utility>

#include "Callback.hpp"
#include "Capture.hpp"

namespace soup
{
	struct Worker
	{
		enum HoldupType : uint8_t
		{
			NONE,
			SOCKET,
			IDLE, // call holdup callback whenever the scheduler has some idle time
			PROMISE,
		};

		bool is_socket;
		uint8_t recursions = 0;
		HoldupType holdup_type = NONE;
		Callback<void(Worker&)> holdup_callback;
		void* holdup_data;

		Worker(bool is_socket = false) noexcept
			: is_socket(is_socket)
		{
		}

		virtual ~Worker() = default;

		void operator=(Worker&& b) noexcept;

		void fireHoldupCallback();
		void awaitPromiseCompletion(PromiseBase* p, void(*f)(Worker&, Capture&&), Capture&& cap);

		[[nodiscard]] bool canRecurse() noexcept;
	};
}

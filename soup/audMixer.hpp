#pragma once

#include "base.hpp"
#if (SOUP_WINDOWS && !SOUP_CROSS_COMPILE) || SOUP_LINUX
#include "fwd.hpp"

#include <vector>

#include "Mutex.hpp"
#include "SharedPtr.hpp"

NAMESPACE_SOUP
{
	class audMixer
	{
	public:
		Mutex mtx{};
		std::vector<SharedPtr<audSound>> playing_sounds{};
		bool stop_playback_when_done = false;

		void setOutput(audPlayback& pb);

		void playSound(SharedPtr<audSound> sound);

	protected:
		[[nodiscard]] double getAmplitude(audPlayback& pb) noexcept;
	};
}

#endif

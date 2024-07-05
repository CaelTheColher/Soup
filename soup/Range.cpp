#include "Range.hpp"

#include "base.hpp"

#if SOUP_X86 && SOUP_BITS == 64
#include <emmintrin.h>
#endif

#if SOUP_WINDOWS
#include <windows.h>
#endif

#include "bitutil.hpp"
#include "CpuInfo.hpp"
#include "Pattern.hpp"

NAMESPACE_SOUP
{
	Range::Range(Pointer base, size_t size) noexcept
		: base(base), size(size)
	{
	}

	Pointer Range::end() const noexcept
	{
		return base.add(size);
	}

	bool Range::contains(Pointer h) const noexcept
	{
		return h.as<uintptr_t>() >= base.as<uintptr_t>() && h.as<uintptr_t>() <= end().as<uintptr_t>();
	}

	bool Range::pattern_matches(uint8_t* target, const std::optional<uint8_t>* sig, size_t length) noexcept
	{
#if SOUP_WINDOWS && !SOUP_CROSS_COMPILE
		__try
		{
#endif
			for (size_t i = 0; i < length; ++i)
			{
				if (sig[i] && *sig[i] != target[i])
				{
					return false;
				}
			}
			return true;
#if SOUP_WINDOWS && !SOUP_CROSS_COMPILE
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
		return false;
#endif
	}

	size_t Range::scanWithMultipleResults(const Pattern& sig, Pointer buf[], size_t buflen) const noexcept
	{
		const auto data = sig.bytes.data();
		const auto length = sig.bytes.size();
		SOUP_IF_UNLIKELY (length == 0)
		{
			buf[0] = base;
			return 1;
		}
#if SOUP_X86 && SOUP_BITS == 64
		SOUP_IF_LIKELY (data[sig.most_unique_byte_index].has_value())
		{
			if (CpuInfo::get().supportsSSE2())
			{
				return scanWithMultipleResultsSimd(sig, buf, buflen);
			}
		}
#endif
		size_t accum = 0;
		for (uintptr_t i = 0; i != (size - length); ++i)
		{
			if (pattern_matches(base.add(i).as<uint8_t*>(), data, length))
			{
				buf[accum++] = base.add(i);
				if (accum == buflen)
				{
					break;
				}
			}
		}
		return accum;
	}

#if SOUP_X86 && SOUP_BITS == 64
	size_t Range::scanWithMultipleResultsSimd(const Pattern& sig, Pointer buf[], size_t buflen) const noexcept
	{
		const auto data = sig.bytes.data();
		const auto length = sig.bytes.size();
		const auto match = _mm_set1_epi8(*data[sig.most_unique_byte_index]);
		size_t accum = 0;
		for (uintptr_t i = sig.most_unique_byte_index; i < (size - length - 15); i += 16)
		{
			uint32_t mask = _mm_movemask_epi8(_mm_cmpeq_epi8(match, _mm_loadu_si128(base.add(i).as<const __m128i*>())));
			while (mask != 0)
			{
				const auto j = bitutil::getLeastSignificantSetBit(mask);
				if (pattern_matches(base.add(i).add(j).sub(sig.most_unique_byte_index).as<uint8_t*>(), data, length))
				{
					buf[accum++] = base.add(i).add(j).sub(sig.most_unique_byte_index);
					if (accum == buflen)
					{
						return buflen;
					}
				}
				bitutil::unsetLeastSignificantSetBit(mask);
			}
		}
		return accum;
	}
#endif
}

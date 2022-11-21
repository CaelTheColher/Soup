#include "Bigint.hpp"

#include "RngInterface.hpp"
#include "Bitset.hpp"
#include "branchless.hpp"
#include "Endian.hpp"
#include "rand.hpp"
#include "string.hpp"

namespace soup
{
	using chunk_t = Bigint::chunk_t;
	using chunk_signed_t = Bigint::chunk_signed_t;

	Bigint::Bigint(chunk_signed_t v)
		: Bigint()
	{
		operator =(v);
	}

	Bigint::Bigint(chunk_t v, bool negative)
		: negative(negative)
	{
		setChunks(v);
	}

	Bigint::Bigint(intmax_t v)
		: Bigint()
	{
		operator =(v);
	}

	Bigint::Bigint(size_t v, bool negative)
		: negative(negative)
	{
		setChunks(v);
	}

	Bigint::Bigint(Bigint&& b)
		: chunks(std::move(b.chunks)), negative(b.negative)
	{
	}

	Bigint::Bigint(const Bigint& b)
		: chunks(b.chunks), negative(b.negative)
	{
	}

	Bigint Bigint::fromString(const char* str, size_t len)
	{
		Bigint res{};
		if (len != 0)
		{
			const bool neg = (str[0] == '-');
			if (neg)
			{
				++str;
				--len;
			}
			if (len > 2 && str[0] == '0')
			{
				if (str[1] == 'b' || str[1] == 'B')
				{
					res.fromStringImplBinary(str + 2, len - 2);
				}
				else if (str[1] == 'x' || str[1] == 'X')
				{
					res.fromStringImplHex(str + 2, len - 2);
				}
				else
				{
					res.fromStringImplDecimal(str, len);
				}
			}
			else
			{
				res.fromStringImplDecimal(str, len);
			}
			res.negative = neg;
		}
		return res;
	}

	Bigint Bigint::fromStringHex(const char* str, size_t len)
	{
		Bigint res{};
		if (len != 0)
		{
			const bool neg = (str[0] == '-');
			if (neg)
			{
				++str;
				--len;
			}
			res.fromStringImplHex(str, len);
			res.negative = neg;
		}
		return res;
	}

	void Bigint::fromStringImplBinary(const char* str, size_t len)
	{
		for (size_t i = 0; i != len; ++i)
		{
			if (str[i] != '0')
			{
				enableBit(len - 1 - i);
			}
		}
	}

	void Bigint::fromStringImplDecimal(const char* str, size_t len)
	{
		for (size_t i = 0; i != len; ++i)
		{
			*this *= Bigint((chunk_t)10u);
			*this += (chunk_t)(str[i] - '0');
		}
	}

	void Bigint::fromStringImplHex(const char* str, size_t len)
	{
		for (size_t i = 0; i != len; ++i)
		{
			*this <<= 4u;
			if (str[i] >= 'a')
			{
				*this |= (chunk_t)(str[i] - ('a' - 10u));
			}
			else if (str[i] >= 'A')
			{
				*this |= (chunk_t)(str[i] - ('A' - 10u));
			}
			else
			{
				*this |= (chunk_t)(str[i] - '0');
			}
		}
	}

	Bigint Bigint::random(size_t bits)
	{
		Bigint res{};
		if ((bits % getBitsPerChunk()) == 0)
		{
			bits /= getBitsPerChunk();
			for (size_t i = 0; i != bits; ++i)
			{
				res.chunks.emplace_back(rand.t<chunk_t>(0, -1));
			}
		}
		else
		{
			for (size_t i = 0; i != bits; ++i)
			{
				if (rand.coinflip())
				{
					res.enableBit(i);
				}
			}
		}
		return res;
	}

	Bigint Bigint::random(RngInterface& rng, size_t bits)
	{
		Bigint res{};
		if ((bits % getBitsPerChunk()) == 0)
		{
			bits /= getBitsPerChunk();
			for (size_t i = 0; i != bits; ++i)
			{
				res.chunks.emplace_back((chunk_t)rng.generate());
			}
		}
		else
		{
			for (size_t i = 0; i != bits; ++i)
			{
				if (rng.coinflip())
				{
					res.enableBit(i);
				}
			}
		}
		return res;
	}

	Bigint Bigint::randomProbablePrime(const size_t bits, const int miller_rabin_iterations)
	{
		Bigint i = random(bits);
		for (; i.enableBitInbounds(0), !i.isProbablePrime(miller_rabin_iterations); i = random(bits));
		return i;
	}

	Bigint Bigint::randomProbablePrime(RngInterface& rng, const size_t bits, const int miller_rabin_iterations)
	{
		Bigint i = random(rng, bits);
		for (; i.enableBitInbounds(0), !i.isProbablePrime(miller_rabin_iterations); i = random(rng, bits));
		return i;
	}

	chunk_t Bigint::getChunk(size_t i) const noexcept
	{
		if (i < getNumChunks())
		{
			return getChunkInbounds(i);
		}
		return 0;
	}

	chunk_t Bigint::getChunkInbounds(size_t i) const noexcept
	{
		return chunks[i];
	}

	void Bigint::setChunk(size_t i, chunk_t v)
	{
		if (i < chunks.size())
		{
			setChunkInbounds(i, v);
		}
		else
		{
			addChunk(v);
		}
	}

	void Bigint::setChunkInbounds(size_t i, chunk_t v)
	{
		chunks[i] = v;
	}

	void Bigint::addChunk(size_t i, chunk_t v)
	{
		while (i != chunks.size())
		{
			addChunk(0);
		}
		addChunk(v);
	}

	void Bigint::addChunk(chunk_t v)
	{
		chunks.emplace_back(v);
	}

	void Bigint::shrink() noexcept
	{
		for (size_t i = chunks.size(); i-- != 0; )
		{
			if (chunks[i] != 0)
			{
				break;
			}
#if SOUP_WINDOWS
			chunks.erase(i);
#else
			chunks.erase(chunks.cbegin() + i);
#endif
		}
	}

	size_t Bigint::getNumBytes() const noexcept
	{
		return getNumChunks() * getBytesPerChunk();
	}

	uint8_t Bigint::getByte(const size_t i) const noexcept
	{
		auto j = i / getBytesPerChunk();
		auto k = i % getBytesPerChunk();

		if (j < chunks.size())
		{
			return (reinterpret_cast<const uint8_t*>(&chunks[j]))[k];
		}
		return 0;
	}

	size_t Bigint::getNumNibbles() const noexcept
	{
		return getNumBytes() * 2;
	}

	uint8_t Bigint::getNibble(size_t i) const noexcept
	{
		return ((getByte(i / 2) >> ((i % 2) * 4)) & 0b1111);
	}

	size_t Bigint::getNumBits() const noexcept
	{
		return getNumChunks() * getBitsPerChunk();
	}

	bool Bigint::getBit(const size_t i) const noexcept
	{
		auto chunk_i = i / getBitsPerChunk();
		auto j = i % getBitsPerChunk();

		return (getChunk(chunk_i) >> j) & 1;
	}

	void Bigint::setBit(const size_t i, const bool v)
	{
		if (v)
		{
			enableBit(i);
		}
		else
		{
			disableBit(i);
		}
	}

	void Bigint::enableBit(const size_t i)
	{
		auto chunk_i = i / getBitsPerChunk();
		auto j = i % getBitsPerChunk();

		j = (1 << j);

		if (chunk_i < chunks.size())
		{
			chunks[chunk_i] |= j;
		}
		else
		{
			addChunk(chunk_i, j);
		}
	}

	void Bigint::disableBit(const size_t i)
	{
		auto chunk_i = i / getBitsPerChunk();
		auto j = i % getBitsPerChunk();

		if (chunk_i < chunks.size())
		{
			chunks[chunk_i] &= ~(1 << j);
		}
	}

	size_t Bigint::getBitLength() const noexcept
	{
		size_t len = getNumBits();
		while (len-- != 0 && !getBit(len));
		return len + 1;
	}

	size_t Bigint::getLowestSetBit() const noexcept
	{
		const auto nb = getNumBits();
		for (size_t i = 0; i != nb; ++i)
		{
			if (getBit(i))
			{
				return i;
			}
		}
		return -1;
	}

	bool Bigint::getBitInbounds(const size_t i) const noexcept
	{
		auto chunk_i = i / getBitsPerChunk();
		auto j = i % getBitsPerChunk();

		return (getChunkInbounds(chunk_i) >> j) & 1;
	}

	void Bigint::setBitInbounds(const size_t i, const bool v)
	{
		auto chunk_i = i / getBitsPerChunk();
		auto j = i % getBitsPerChunk();

		Bitset<chunk_t>::at(chunks[chunk_i]).set(j, v);
	}

	void Bigint::enableBitInbounds(const size_t i)
	{
		auto chunk_i = i / getBitsPerChunk();
		auto j = i % getBitsPerChunk();

		chunks[chunk_i] |= (1 << j);
	}

	void Bigint::disableBitInbounds(const size_t i)
	{
		auto chunk_i = i / getBitsPerChunk();
		auto j = i % getBitsPerChunk();

		chunks[chunk_i] &= ~(1 << j);
	}

	void Bigint::reset() noexcept
	{
		chunks.clear();
		negative = false;
	}

	bool Bigint::isZero() const noexcept
	{
		return getNumChunks() == 0;
	}

	Bigint::operator bool() const noexcept
	{
		return !isZero();
	}

	int Bigint::cmp(const Bigint& b) const noexcept
	{
		if (getNumChunks() != b.getNumChunks())
		{
			return branchless::trinary(getNumChunks() > b.getNumChunks(), +1, -1);
		}
		if (negative)
		{
			if (!b.negative)
			{
				return -1;
			}
		}
		else
		{
			if (b.negative)
			{
				return +1;
			}
		}
		size_t i = chunks.size();
		while (i != 0)
		{
			--i;
			if (getChunkInbounds(i) != b.getChunkInbounds(i))
			{
				return branchless::trinary(getChunkInbounds(i) > b.getChunkInbounds(i), +1, -1);
			}
		}
		return 0;
	}

	int Bigint::cmpUnsigned(const Bigint& b) const noexcept
	{
		if (getNumChunks() != b.getNumChunks())
		{
			return branchless::trinary(getNumChunks() > b.getNumChunks(), +1, -1);
		}
		size_t i = chunks.size();
		while (i != 0)
		{
			--i;
			if (getChunk(i) != b.getChunk(i))
			{
				return branchless::trinary(getChunk(i) > b.getChunk(i), +1, -1);
			}
		}
		return 0;
	}

	bool Bigint::operator==(const Bigint& b) const noexcept
	{
		return cmp(b) == 0;
	}

	bool Bigint::operator!=(const Bigint& b) const noexcept
	{
		return cmp(b) != 0;
	}

	bool Bigint::operator>(const Bigint& b) const noexcept
	{
		return cmp(b) > 0;
	}

	bool Bigint::operator>=(const Bigint& b) const noexcept
	{
		return cmp(b) >= 0;
	}

	bool Bigint::operator<(const Bigint& b) const noexcept
	{
		return cmp(b) < 0;
	}

	bool Bigint::operator<=(const Bigint& b) const noexcept
	{
		return cmp(b) <= 0;
	}

	bool Bigint::operator==(const chunk_t v) const noexcept
	{
		return !negative && getNumChunks() == 1 && getChunk(0) == v;
	}

	bool Bigint::operator!=(const chunk_t v) const noexcept
	{
		return !operator==(v);
	}

	bool Bigint::operator<(const chunk_t v) const noexcept
	{
		return negative || (getNumChunks() == 0 && v != 0) || (getNumChunks() == 1 && getChunk(0) < v);
	}

	bool Bigint::operator<=(const chunk_t v) const noexcept
	{
		return negative || getNumChunks() == 0 || (getNumChunks() == 1 && getChunk(0) <= v);
	}

	void Bigint::operator=(chunk_signed_t v)
	{
		negative = (v < 0);
		if (negative)
		{
			setChunks((chunk_t)(v * -1));
		}
		else
		{
			setChunks((chunk_t)v);
		}
	}

	void Bigint::operator=(chunk_t v)
	{
		setChunks(v);
		negative = false;
	}

	void Bigint::operator=(intmax_t v)
	{
		negative = (v < 0);
		if (negative)
		{
			setChunks((size_t)(v * -1));
		}
		else
		{
			setChunks((size_t)v);
		}
	}

	void Bigint::operator=(size_t v)
	{
		setChunks(v);
		negative = false;
	}

	void Bigint::setChunks(chunk_t v)
	{
		chunks.clear();
		if (v != 0)
		{
			chunks.emplace_back(v);
		}
	}

	void Bigint::setChunks(size_t v)
	{
		const chunk_t carry = getCarry(v);
		if (carry == 0)
		{
			setChunks((chunk_t)v);
		}
		else
		{
#if SOUP_WINDOWS
			chunks.clear();
			chunks.emplace_back((chunk_t)v);
			chunks.emplace_back(carry);
#else
			chunks = std::vector<chunk_t>{ (chunk_t)v, carry };
#endif
		}
	}

	void Bigint::operator=(Bigint&& b)
	{
		chunks = std::move(b.chunks);
		negative = b.negative;
	}

	void Bigint::operator=(const Bigint& b)
	{
		chunks = b.chunks;
		negative = b.negative;
	}

	void Bigint::operator+=(const Bigint& b)
	{
		if (negative ^ b.negative)
		{
			subUnsigned(b);
		}
		else
		{
			addUnsigned(b);
		}
	}

	void Bigint::addUnsigned(const Bigint& b)
	{
		if (cmp(b) < 0)
		{
			Bigint res(b);
			res.addUnsigned(*this);
			chunks = std::move(res.chunks);
			return;
		}
		chunk_t carry = 0;
		const size_t j = getNumChunks();
		for (size_t i = 0; i != j; ++i)
		{
			const size_t x = getChunkInbounds(i);
			const size_t y = b.getChunk(i);
			size_t res = (x + y + carry);
			setChunkInbounds(i, (chunk_t)res);
			carry = getCarry(res);
		}
		if (carry != 0)
		{
			addChunk(carry);
		}
	}

	void Bigint::operator-=(const Bigint& subtrahend)
	{
		if (negative ^ subtrahend.negative)
		{
			addUnsigned(subtrahend);
		}
		else
		{
			subUnsigned(subtrahend);
		}
	}

	void Bigint::subUnsigned(const Bigint& subtrahend)
	{
		if (cmpUnsigned(subtrahend) == 0)
		{
			reset();
			return;
		}
		if (cmp(subtrahend) < 0)
		{
			Bigint res(subtrahend);
			res.subUnsigned(*this);
			chunks = std::move(res.chunks);
			negative ^= 1;
			return;
		}
		size_t carry = 0;
		const size_t j = getNumChunks();
		for (size_t i = 0; i != j; ++i)
		{
			const size_t x = getChunkInbounds(i);
			const size_t y = subtrahend.getChunk(i);
			size_t res = (x - y - carry);
			setChunkInbounds(i, (chunk_t)res);
			carry = (getCarry(res) != 0);
		}
		shrink();
	}

	void Bigint::operator*=(const Bigint& b)
	{
		*this = (*this * b);
	}

	void Bigint::operator/=(const Bigint& divisor)
	{
		*this = divide(divisor).first;
	}

	void Bigint::operator%=(const Bigint& divisor)
	{
		if (*this >= divisor)
		{
			*this = divide(divisor).second;
		}
	}

	std::pair<Bigint, Bigint> Bigint::divide(const Bigint& divisor) const
	{
		if (divisor.negative)
		{
			Bigint divisor_cpy(divisor);
			divisor_cpy.negative = false;
			auto res = divide(divisor_cpy);
			res.first.negative ^= 1;
			return res;
		}
		if (negative)
		{
			Bigint dividend(*this);
			dividend.negative = false;
			auto res = dividend.divide(divisor);
			res.first.negative ^= 1;
			if (!res.second.isZero())
			{
				res.first -= Bigint((chunk_t)1u);
				res.second = divisor - res.second;
			}
			return res;
		}
		return divideUnsigned(divisor);
	}

	std::pair<Bigint, Bigint> Bigint::divideUnsigned(const Bigint& divisor) const
	{
		std::pair<Bigint, Bigint> res{};
		if (!divisor.isZero())
		{
			if (divisor == Bigint((chunk_t)2u))
			{
				res.first = *this;
				res.first >>= 1u;
				res.second = (chunk_t)getBit(0);
			}
			else
			{
				for (size_t i = getNumBits(); i-- != 0; )
				{
					res.second.leftShiftSmall(1);
					res.second.setBit(0, getBitInbounds(i));
					if (res.second >= divisor)
					{
						res.second -= divisor;
						res.first.enableBit(i);
					}
				}
			}
		}
		return res;
	}

	Bigint Bigint::mod(const Bigint& m) const
	{
		return divide(m).second;
	}

	Bigint Bigint::modUnsigned(const Bigint& m) const
	{
		auto m_minus_1 = (m - Bigint((chunk_t)1u));
		if ((m & m_minus_1).isZero())
		{
			return (*this & m_minus_1);
		}
		return modUnsignedNotpowerof2(m);
	}

	Bigint Bigint::modUnsignedPowerof2(const Bigint& m) const
	{
		return (*this & (m - Bigint((chunk_t)1u)));
	}

	Bigint Bigint::modUnsignedNotpowerof2(const Bigint& divisor) const
	{
		Bigint remainder{};
		for (size_t i = getNumBits(); i-- != 0; )
		{
			remainder.leftShiftSmall(1);
			remainder.setBit(0, getBitInbounds(i));
			if (remainder >= divisor)
			{
				remainder.subUnsigned(divisor);
			}
		}
		return remainder;
	}

	bool Bigint::isDivisorOf(const Bigint& dividend) const
	{
		return (dividend % *this).isZero();
	}

	void Bigint::operator<<=(const size_t b)
	{
		if (b <= getBitsPerChunk())
		{
			return leftShiftSmall(b);
		}

		const auto nb = getNumBits();
		if (nb != 0)
		{
			const auto maxbitidx = nb - 1;
			// extend
			{
				size_t i = maxbitidx;
				for (size_t j = 0; j != b; ++j)
				{
					setBit(i + b, getBitInbounds(i));
					if (i-- == 0)
					{
						break;
					}
				}
			}

			// move
			if (b < nb)
			{
				for (size_t i = nb; i-- != b; )
				{
					setBitInbounds(i, getBitInbounds(i - b));
				}
			}

			// disable
			if (b < getNumBits())
			{
				for (size_t i = 0; i != b; ++i)
				{
					disableBitInbounds(i);
				}
			}
		}
	}

	void Bigint::leftShiftSmall(const size_t b)
	{
		size_t carry = 0;
		const auto nc = getNumChunks();
		for (size_t i = 0; i != nc; ++i)
		{
			if constexpr (NATIVE_ENDIAN == LITTLE_ENDIAN)
			{
				chunk_t c[2];
				c[0] = getChunkInbounds(i);
				c[1] = 0;
				*reinterpret_cast<size_t*>(&c[0]) = ((*reinterpret_cast<size_t*>(&c[0]) << b) | carry);
				setChunkInbounds(i, c[0]);
				carry = c[1];
			}
			else
			{
				size_t c = getChunkInbounds(i);
				c = ((c << b) | carry);
				setChunkInbounds(i, c);
				carry = getCarry(c);
			}
		}
		if (carry != 0)
		{
			addChunk(carry);
		}
	}

	void Bigint::operator>>=(const size_t b)
	{
		if constexpr (NATIVE_ENDIAN == LITTLE_ENDIAN)
		{
			if (b <= getBitsPerChunk())
			{
				const auto nc = getNumChunks();
				if (nc != 0)
				{
					chunk_t carry = 0;
					for (size_t i = nc; i-- != 0; )
					{
						chunk_t c[2];
						c[0] = 0;
						c[1] = getChunkInbounds(i);
						*reinterpret_cast<size_t*>(&c[0]) = (*reinterpret_cast<size_t*>(&c[0]) >> b);
						setChunkInbounds(i, c[1] | carry);
						carry = c[0];
					}
					shrink();
				}
				return;
			}
		}

		size_t nb = getNumBits();
		if (nb != 0)
		{
			for (size_t i = 0; i != nb; ++i)
			{
				setBitInbounds(i, getBit(i + b));
			}
			for (size_t i = nb, j = 0; --i, j != b; ++j)
			{
				disableBitInbounds(i);
			}
			shrink();
		}
	}

	void Bigint::operator|=(const Bigint& b)
	{
		const auto nb = b.getNumBits();
		for (size_t i = 0; i != nb; ++i)
		{
			if (b.getBit(i))
			{
				enableBit(i);
			}
		}
	}

	void Bigint::operator&=(const Bigint& b)
	{
		const auto nc = getNumChunks();
		for (size_t i = 0; i != nc; ++i)
		{
			setChunkInbounds(i, getChunkInbounds(i) & b.getChunk(i));
		}
		shrink();
	}

	Bigint Bigint::operator+(const Bigint& b) const
	{
		Bigint res(*this);
		res += b;
		return res;
	}

	// ++a
	Bigint& Bigint::operator++()
	{
		*this += Bigint((chunk_t)1u);
		return *this;
	}

	// a++
	Bigint Bigint::operator++(int)
	{
		Bigint pre(*this);
		++*this;
		return pre;
	}

	Bigint Bigint::operator-(const Bigint& subtrahend) const
	{
		Bigint res(*this);
		res -= subtrahend;
		return res;
	}

	Bigint& Bigint::operator--()
	{
		*this -= Bigint((chunk_t)1u);
		return *this;
	}

	Bigint Bigint::operator--(int)
	{
		Bigint pre(*this);
		--*this;
		return pre;
	}

	Bigint Bigint::operator*(const Bigint& b) const
	{
		Bigint product{};
		if (!isZero() && !b.isZero())
		{
			product.negative = (negative ^ b.negative);
			const auto nc = getNumChunks();
			const auto b_nc = b.getNumChunks();
			for (size_t j = 0; j != b_nc; ++j)
			{
				chunk_t carry = 0;
				const size_t y = b.getChunkInbounds(j);
				for (size_t i = 0; i != nc; ++i)
				{
					const size_t x = getChunkInbounds(i);
					size_t res = product.getChunk(i + j) + (x * y) + carry;
					product.setChunk(i + j, (chunk_t)res);
					carry = getCarry(res);
				}
				if (carry != 0)
				{
					product.setChunk(j + nc, carry);
				}
			}
		}
		return product;
	}

	Bigint Bigint::operator/(const Bigint& b) const
	{
		return divide(b).first;
	}

	Bigint Bigint::operator%(const Bigint& b) const
	{
		return divide(b).second;
	}

	Bigint Bigint::operator<<(size_t b) const
	{
		Bigint res(*this);
		res <<= b;
		return res;
	}

	Bigint Bigint::operator>>(size_t b) const
	{
		Bigint res(*this);
		res >>= b;
		return res;
	}

	Bigint Bigint::operator|(const Bigint& b) const
	{
		Bigint res(*this);
		res |= b;
		return res;
	}

	Bigint Bigint::operator&(const Bigint& b) const
	{
		Bigint res(*this);
		res &= b;
		return res;
	}

	bool Bigint::isNegative() const noexcept
	{
		return negative;
	}
	
	bool Bigint::isEven() const noexcept
	{
		return !isOdd();
	}

	bool Bigint::isOdd() const noexcept
	{
		return getBit(0);
	}

	Bigint Bigint::abs() const
	{
		Bigint res(*this);
		res.negative = false;
		return res;
	}

	Bigint Bigint::pow(Bigint e) const
	{
		if (*this == Bigint((chunk_t)2u))
		{
			size_t e_primitive;
			if (e.toPrimitive(e_primitive))
			{
				return _2pow(e_primitive);
			}
		}
		return powNot2(e);
	}

	Bigint Bigint::powNot2(Bigint e) const
	{
		Bigint res = Bigint((chunk_t)1u);
		Bigint base(*this);
		while (true)
		{
			if (e.getBit(0))
			{
				res *= base;
			}
			e >>= 1u;
			if (e.isZero())
			{
				break;
			}
			base *= base;
		}
		return res;
	}

	Bigint Bigint::pow2() const
	{
		return *this * *this;
	}

	Bigint Bigint::_2pow(size_t e)
	{
		return Bigint((chunk_t)1u) << e;
	}

	size_t Bigint::getTrailingZeroes(const Bigint& base) const
	{
		if (base == Bigint((chunk_t)2u))
		{
			return getTrailingZeroesBinary();
		}
		size_t res = 0;
		Bigint tmp(*this);
		while (!tmp.isZero())
		{
			auto pair = tmp.divide(base);
			if (!pair.second.isZero())
			{
				break;
			}
			++res;
			tmp = std::move(pair.first);
		}
		return res;
	}

	size_t Bigint::getTrailingZeroesBinary() const
	{
		size_t res = 0;
		const auto nb = getNumBits();
		for (size_t i = 0; i != nb && !getBit(i); ++i)
		{
			++res;
		}
		return res;
	}

	Bigint Bigint::gcd(Bigint v) const
	{
		Bigint u(*this);

		auto i = u.getTrailingZeroesBinary(); u >>= i;
		auto j = v.getTrailingZeroesBinary(); v >>= j;
		auto k = branchless::min(i, j);

		while (true)
		{
			if (u > v)
			{
				std::swap(u, v);
			}

			v -= u;

			if (v.isZero())
			{
				u <<= k;
				return u;
			}

			v >>= v.getTrailingZeroesBinary();
		}
	}

	Bigint Bigint::gcd(Bigint b, Bigint& x, Bigint& y) const
	{
		if (isZero())
		{
			//x.reset();
			y = Bigint((chunk_t)1u);
			return b;
		}
		auto d = b.divide(*this);
		Bigint xr, yr;
		auto g = d.second.gcd(*this, xr, yr);
		x = (yr - (d.first * xr));
		y = std::move(xr);
		return g;
	}

	bool Bigint::isPrimePrecheck(bool& ret) const
	{
		if (isZero() || *this == Bigint((chunk_t)1u))
		{
			ret = false;
			return true;
		}
		if (*this <= Bigint((chunk_t)3u))
		{
			ret = true;
			return true;
		}
		if (getBit(0) == 0)
		{
			ret = false;
			return true;
		}

		if (this->modUnsigned(Bigint((chunk_t)3u)).isZero())
		{
			ret = false;
			return true;
		}
		return false;
	}

	bool Bigint::isPrime() const
	{
		bool preret;
		if (isPrimePrecheck(preret))
		{
			return preret;
		}

		if (getNumBits() < 128)
		{
			return isPrimeAccurateNoprecheck();
		}

		return isProbablePrimeNoprecheck(10);
	}

	bool Bigint::isPrimeAccurate() const
	{
		bool preret;
		if (isPrimePrecheck(preret))
		{
			return preret;
		}

		return isPrimeAccurateNoprecheck();
	}

	bool Bigint::isPrimeAccurateNoprecheck() const
	{
		for (Bigint i = Bigint((chunk_t)5u); i * i <= *this; i += Bigint((chunk_t)6u))
		{
			if ((*this % i).isZero() || (*this % (i + Bigint((chunk_t)2u))).isZero())
			{
				return false;
			}
		}
		return true;
	}

	bool Bigint::isProbablePrime(const int miller_rabin_iterations) const
	{
		bool preret;
		if (isPrimePrecheck(preret))
		{
			return preret;
		}

		return isProbablePrimeNoprecheck(miller_rabin_iterations);
	}

	bool Bigint::isProbablePrimeNoprecheck(const int miller_rabin_iterations) const
	{
		auto thisMinusOne = (*this - Bigint((chunk_t)1u));
		auto a = thisMinusOne.getLowestSetBit();
		auto m = (thisMinusOne >> a);

		const auto bl = getBitLength();
		for (int i = 0; i != miller_rabin_iterations; i++)
		{
			Bigint b;
			do
			{
				b = random(bl);
			} while (b >= *this || b <= Bigint((chunk_t)1u));

			int j = 0;
			Bigint z = b.modPow(m, *this);
			while (!((j == 0 && z == Bigint((chunk_t)1u)) || z == thisMinusOne))
			{
				if ((j > 0 && z == Bigint((chunk_t)1u)) || ++j == a)
				{
					return false;
				}
				// z = z.modPow(2u, *this);
				z = z.modMulUnsignedNotpowerof2(z, *this);
			}
		}
		return true;
	}

	bool Bigint::isCoprime(const Bigint& b) const
	{
		return gcd(b) == Bigint((chunk_t)1u);
	}

	Bigint Bigint::eulersTotient() const
	{
		Bigint res = Bigint((chunk_t)1u);
		for (Bigint i = Bigint((chunk_t)2u); i != *this; ++i)
		{
			if (isCoprime(i))
			{
				++res;
			}
		}
		return res;
	}

	Bigint Bigint::reducedTotient() const
	{
		if (*this <= Bigint((chunk_t)2u))
		{
			return Bigint((chunk_t)1u);
		}
		std::vector<Bigint> coprimes{};
		for (Bigint a = Bigint((chunk_t)2u); a != *this; ++a)
		{
			if (isCoprime(a))
			{
				coprimes.emplace_back(a);
			}
		}
		Bigint k = Bigint((chunk_t)2u);
		for (auto timer = coprimes.size(); timer != 0; )
		{
			for (auto i = coprimes.begin(); i != coprimes.end(); ++i)
			{
				if (i->modPow(k, *this) == Bigint((chunk_t)1u))
				{
					if (timer == 0)
					{
						break;
					}
					--timer;
				}
				else
				{
					timer = coprimes.size();
					++k;
				}
			}
		}
		return k;
	}

	Bigint Bigint::lcm(const Bigint& b) const
	{
		if (isZero() || b.isZero())
		{
			return Bigint();
		}
		auto a_mag = abs();
		auto b_mag = b.abs();
		return ((a_mag * b_mag) / a_mag.gcd(b_mag));
	}

	bool Bigint::isPowerOf2() const
	{
		return (*this & (*this - Bigint((chunk_t)1u))).isZero();
	}

	std::pair<Bigint, Bigint> Bigint::factorise() const
	{
		{
			auto [q, r] = divide((chunk_t)2u);
			if (r.isZero())
			{
				return { q, (chunk_t)2u };
			}
		}

		Bigint a = sqrtCeil();

		if ((a * a) == *this)
		{
			return { a, a };
		}

		Bigint b;
		const Bigint one = (chunk_t)1u;
		while (true)
		{
			Bigint b1 = (a * a) - *this;
			b = b1.sqrtFloor();
			if ((b * b) == b1)
			{
				break;
			}
			a += one;
		}
		return { a - b, a + b };
	}

	Bigint Bigint::sqrtCeil() const
	{
		Bigint y = sqrtFloor();

		if (*this == (y * y))
		{
			return y;
		}

		return (y + Bigint((chunk_t)1u));
	}

	Bigint Bigint::sqrtFloor() const
	{
		if (isZero() || *this == (chunk_t)1u)
		{
			return *this;
		}

		const Bigint two = (chunk_t)2u;
		Bigint y = (*this / two);
		Bigint x_over_y;

		while (x_over_y = (*this / y), y > x_over_y)
		{
			y = (x_over_y + y) / two;
		}

		return y;
	}

	Bigint Bigint::modMulInv(const Bigint& m) const
	{
		Bigint x, y;
		if (gcd(m, x, y) == Bigint((chunk_t)1u))
		{
			return (x % m + m) % m;
		}

		for (Bigint res = Bigint((chunk_t)1u);; ++res)
		{
			if (((*this * res) % m) == Bigint((chunk_t)1u))
			{
				return res;
			}
		}
	}

	void Bigint::modMulInv2Coprimes(const Bigint& a, const Bigint& m, Bigint& x, Bigint& y)
	{
		a.gcd(m, x, y);
		x = ((x % m + m) % m);
		y = ((y % a + a) % a);
	}

	Bigint Bigint::modMulUnsigned(const Bigint& b, const Bigint& m) const
	{
		return (*this * b).modUnsigned(m);
	}

	Bigint Bigint::modMulUnsignedNotpowerof2(const Bigint& b, const Bigint& m) const
	{
		return (*this * b).modUnsignedNotpowerof2(m);
	}

	Bigint Bigint::modPow(Bigint e, const Bigint& m) const
	{
		if (m.isOdd()
			&& e.getNumBits() > 32 // arbitrary choice
			)
		{
			return modPowMontgomery(e, m);
		}
		return modPowBasic(e, m);
	}

	Bigint Bigint::modPowMontgomery(Bigint e, const Bigint& m) const
	{
		auto re = m.montgomeryREFromM();
		auto r = montgomeryRFromRE(re);
		Bigint m_mod_mul_inv, r_mod_mul_inv;
		modMulInv2Coprimes(m, r, m_mod_mul_inv, r_mod_mul_inv);
		return modPowMontgomery(std::move(e), re, r, m, r_mod_mul_inv, m_mod_mul_inv, r.modUnsignedNotpowerof2(m));
	}

	Bigint Bigint::modPowMontgomery(Bigint e, size_t re, const Bigint& r, const Bigint& m, const Bigint& r_mod_mul_inv, const Bigint& m_mod_mul_inv, const Bigint& one_mont) const
	{
		Bigint res = one_mont;
		Bigint base = enterMontgomerySpace(r, m);
		/*bigint base(*this);
		if (base >= m)
		{
			base = base.modUnsigned(m);
		}
		base = base.enterMontgomerySpace(r, m);*/
		while (!e.isZero())
		{
			if (e.getBit(0))
			{
				res = res.montgomeryMultiplyEfficient(base, r, re, m, m_mod_mul_inv);
			}
			base = base.montgomeryMultiplyEfficient(base, r, re, m, m_mod_mul_inv);
			e >>= 1u;
		}
		return res.leaveMontgomerySpaceEfficient(r_mod_mul_inv, m);
	}

	Bigint Bigint::modPowBasic(Bigint e, const Bigint& m) const
	{
		Bigint base(*this);
		if (base >= m)
		{
			base = base.modUnsigned(m);
		}
		Bigint res = Bigint((chunk_t)1u);
		while (!e.isZero())
		{
			if (e.getBit(0))
			{
				res = res.modMulUnsignedNotpowerof2(base, m);
			}
			base = base.modMulUnsignedNotpowerof2(base, m);
			e >>= 1u;
		}
		return res;
	}

	// We need a positive integer r such that r >= m && r.isCoprime(m)
	// We assume an odd modulus, so any power of 2 will be coprime to it.

	size_t Bigint::montgomeryREFromM() const
	{
		return getBitLength();
	}

	Bigint Bigint::montgomeryRFromRE(size_t re)
	{
		return _2pow(re);
	}

	Bigint Bigint::montgomeryRFromM() const
	{
		return montgomeryRFromRE(montgomeryREFromM());
	}

	Bigint Bigint::enterMontgomerySpace(const Bigint& r, const Bigint& m) const
	{
		return modMulUnsignedNotpowerof2(r, m);
	}

	Bigint Bigint::leaveMontgomerySpace(const Bigint& r, const Bigint& m) const
	{
		return leaveMontgomerySpaceEfficient(r.modMulInv(m), m);
	}

	Bigint Bigint::leaveMontgomerySpaceEfficient(const Bigint& r_mod_mul_inv, const Bigint& m) const
	{
		return modMulUnsignedNotpowerof2(r_mod_mul_inv, m);
	}

	Bigint Bigint::montgomeryMultiply(const Bigint& b, const Bigint& r, const Bigint& m) const
	{
		return (*this * b).montgomeryReduce(r, m);
	}

	Bigint Bigint::montgomeryMultiplyEfficient(const Bigint& b, const Bigint& r, size_t re, const Bigint& m, const Bigint& m_mod_mul_inv) const
	{
		return (*this * b).montgomeryReduce(r, re, m, m_mod_mul_inv);
	}

	Bigint Bigint::montgomeryReduce(const Bigint& r, const Bigint& m) const
	{
		return montgomeryReduce(r, m, m.modMulInv(r));
	}

	Bigint Bigint::montgomeryReduce(const Bigint& r, const Bigint& m, const Bigint& m_mod_mul_inv) const
	{
		auto q = (modUnsignedPowerof2(r) * m_mod_mul_inv).modUnsignedPowerof2(r);
		auto a = (*this - q * m) / r;
		if (a.negative)
		{
			a += m;
		}
		return a;
	}

	Bigint Bigint::montgomeryReduce(const Bigint& r, size_t re, const Bigint& m, const Bigint& m_mod_mul_inv) const
	{
		auto q = (modUnsignedPowerof2(r) * m_mod_mul_inv).modUnsignedPowerof2(r);
		auto a = (*this - q * m);
		a >>= re;
		if (a.negative)
		{
			a += m;
		}
		return a;
	}

	bool Bigint::toPrimitive(size_t& out) const
	{
		switch (getNumChunks())
		{
		case 0:
			out = 0;
			return true;

		case 1:
			out = getChunk(0);
			return true;

		case 2:
			*(chunk_t*)&out = getChunk(0);
			*((chunk_t*)&out + 1) = getChunk(1);
			return true;
		}
		return false;
	}

	std::string Bigint::toStringHex(bool prefix) const
	{
		return toStringHexImpl(prefix, string::charset_hex);
	}

	std::string Bigint::toStringHexLower(bool prefix) const
	{
		return toStringHexImpl(prefix, string::charset_hex_lower);
	}

	std::string Bigint::toStringHexImpl(bool prefix, const char* map) const
	{
		std::string str{};
		size_t i = getNumNibbles();
		if (i == 0)
		{
			str.push_back('0');
		}
		else
		{
			// skip leading zeroes
			while (i-- != 0 && getNibble(i) == 0);
			str.reserve(i + 1 + (prefix * 2) + negative);
			do
			{
				str.push_back(map[getNibble(i)]);
			} while (i-- != 0);
		}
		if (prefix)
		{
			str.insert(0, 1, 'x');
			str.insert(0, 1, '0');
		}
		if (negative)
		{
			str.insert(0, 1, '-');
		}
		return str;
	}

	Bigint Bigint::fromBinary(const std::string& msg)
	{
		Bigint res{};
		for (auto i = msg.begin(); i != msg.end(); ++i)
		{
			res <<= 8u;
			res |= (chunk_t)(unsigned char)*i;
		}
		return res;
	}

	std::string Bigint::toBinary() const
	{
		std::string str{};
		size_t i = getNumBytes();
		if (i != 0)
		{
			// skip leading zeroes
			while (i-- != 0 && getByte(i) == 0);
			str.reserve(i);
			do
			{
				str.push_back(getByte(i));
			} while (i-- != 0);
		}
		return str;
	}
}

#include "Regex.hpp"

#include "base.hpp"
#include "RegexConstraintTransitionable.hpp"
#include "RegexMatcher.hpp"

#define REGEX_DEBUG_MATCH false

#if REGEX_DEBUG_MATCH
#include <iostream>
#endif

namespace soup
{
	bool Regex::matches(const std::string& str) const noexcept
	{
		return matches(str.cbegin(), str.cend());
	}

	bool Regex::matches(std::string::const_iterator it, std::string::const_iterator end) const noexcept
	{
		return match(it, end).isSuccess();
	}

	bool Regex::matchesFully(const std::string& str) const noexcept
	{
		return matchesFully(str.cbegin(), str.cend());
	}

	bool Regex::matchesFully(std::string::const_iterator it, std::string::const_iterator end) const noexcept
	{
		auto res = match(it, end);
		if (res.isSuccess())
		{
			return res.groups.at(0)->end == end;
		}
		return false;
	}

	RegexMatchResult Regex::match(const std::string& str) const noexcept
	{
		return match(str.cbegin(), str.cend());
	}

	RegexMatchResult Regex::match(std::string::const_iterator it, std::string::const_iterator end) const noexcept
	{
		RegexMatcher m(*this, it, end);
		while (m.c != nullptr)
		{
#if REGEX_DEBUG_MATCH
			std::cout << m.c->toString() << ": ";
#endif

			if (m.c->rollback_transition)
			{
#if REGEX_DEBUG_MATCH
				std::cout << "saved rollback; ";
#endif
				m.saveRollback(m.c->rollback_transition);
			}

			// Matches?
			auto _it = m.it;
			if (m.c->matches(m))
			{
				for (auto g = m.c->group; g; g = g->parent)
				{
					if (g->lookahead)
					{
						break;
					}
					if (g->index == -1)
					{
						continue;
					}
					//std::cout << "group " << g->index << "; ";
					while (g->index >= m.groups.size())
					{
						m.groups.emplace_back(std::nullopt);
					}
					if (m.groups.at(g->index).has_value())
					{
						m.groups.at(g->index)->end = m.it;
					}
					else
					{
						m.groups.at(g->index) = RegexMatchedGroup{ g->name, _it, m.it };
					}
				}

				m.c = m.c->success_transition;
				SOUP_ASSERT(reinterpret_cast<uintptr_t>(m.c) != 1);
				if (reinterpret_cast<uintptr_t>(m.c) & 1)
				{
#if REGEX_DEBUG_MATCH
					std::cout << "saved checkpoint; ";
#endif
					m.c = reinterpret_cast<const RegexConstraintTransitionable*>(reinterpret_cast<uintptr_t>(m.c) & ~1);
					m.saveCheckpoint();
				}
#if REGEX_DEBUG_MATCH
				std::cout << "matched\n";
#endif
				continue;
			}

			// Rollback?
			if (!m.rollback_points.empty())
			{
#if REGEX_DEBUG_MATCH
				std::cout << "did not match, rolling back\n";
#endif
				m.restoreRollback();
				if (reinterpret_cast<uintptr_t>(m.c) == 1)
				{
#if REGEX_DEBUG_MATCH
					std::cout << "rollback says we should succeed now\n";
#endif
					goto _match_success;
				}
				continue;
			}

			// Oh well
#if REGEX_DEBUG_MATCH
			std::cout << "no matchy\n";
#endif
			return {};
		}
	_match_success:
		RegexMatchResult res;
		res.groups = std::move(m.groups);

		// Handle match of empty regex
		SOUP_IF_UNLIKELY (!res.isSuccess())
		{
			res.groups.emplace_back(RegexMatchedGroup{ {}, m.begin, m.it });
		}

		SOUP_ASSERT(m.checkpoints.empty()); // if we made a checkpoint for a lookahead group, it should have been restored.

		return res;
	}

	std::string Regex::unparseFlags(uint16_t flags)
	{
		std::string str{};
		if (flags & RE_DOTALL)
		{
			str.push_back('s');
		}
		if (flags & RE_MULTILINE)
		{
			str.push_back('m');
		}
		if (flags & RE_DOLLAR_ENDONLY)
		{
			str.push_back('D');
		}
		return str;
	}
}

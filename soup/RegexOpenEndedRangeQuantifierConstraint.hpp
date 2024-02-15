#pragma once

#include "RegexConstraintTransitionable.hpp"

#include <vector>

#include "UniquePtr.hpp"

namespace soup
{
	struct RegexOpenEndedRangeQuantifierConstraintBase : public RegexConstraintTransitionable
	{
		std::vector<UniquePtr<RegexConstraint>> constraints;

		[[nodiscard]] const RegexConstraintTransitionable* getTransition() const noexcept final
		{
			return constraints.at(0)->getTransition();
		}

		[[nodiscard]] bool matches(RegexMatcher& m) const noexcept final
		{
			// Meta-constraint. Transitions will be set up to correctly handle matching of this.
			return true;
		}

		[[nodiscard]] size_t getCursorAdvancement() const final
		{
			return constraints.at(0)->getCursorAdvancement() * constraints.size();
		}
	};

	template <bool greedy>
	struct RegexOpenEndedRangeQuantifierConstraint : public RegexOpenEndedRangeQuantifierConstraintBase
	{
		[[nodiscard]] std::string toString() const noexcept final
		{
			std::string str = constraints.at(0)->toString();
			str.push_back('{');
			str.append(std::to_string(constraints.size()));
			str.append(",}");
			if (!greedy)
			{
				str.push_back('?');
			}
			return str;
		}
	};
}

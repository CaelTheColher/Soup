#pragma once

#include "irModule.hpp"

NAMESPACE_SOUP
{
	struct laBackend
	{
		virtual void linkPosix(irModule& m) = 0;
		[[nodiscard]] virtual std::string compileModule(const irModule& m) const = 0;
	};
}

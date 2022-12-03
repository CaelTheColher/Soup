#pragma once

#include "cbCmd.hpp"

namespace soup
{
	struct cbCmdDelete : public cbCmd
	{
		[[nodiscard]] std::vector<std::string> getTriggers() const noexcept final
		{
			return { "delete" };
		}

		[[nodiscard]] cbResult process(cbParser& p) const noexcept final
		{
			auto s = p.getArgNumeric();
			long i;
			try
			{
				i = std::stol(s);
			}
			catch (...)
			{
				return "Delete ...?";
			}
			cbResult res(CB_RES_DELETE);
			res.delete_args.num = i;
			return res;
		}
	};
}
#pragma once

#include "fwd.hpp"

#include <vector>

#include "ZipIndexedFile.hpp"

namespace soup
{
	struct ZipReader
	{
		Reader& is;

		ZipReader(Reader& is);

		[[nodiscard]] size_t seekCentralDirectory() const; // returns 0 on failure

		[[nodiscard]] std::vector<ZipIndexedFile> getFileList() const;

		[[nodiscard]] std::string getFileContents(const ZipIndexedFile& file) const;
		[[nodiscard]] std::string getFileContents(uint32_t offset, uint32_t compressed_size = 0) const;
	};
}

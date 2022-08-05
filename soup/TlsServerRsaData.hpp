#pragma once

#include <string>
#include <vector>

#include "rsa.hpp"

namespace soup
{
	struct TlsServerRsaData
	{
		std::vector<std::string> der_encoded_certchain{};
		rsaPrivateKey private_key{};
	};
}

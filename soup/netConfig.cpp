#include "netConfig.hpp"

#if !SOUP_WASM

#include "dnsOsResolver.hpp"
#include "Socket.hpp"

namespace soup
{
	static thread_local netConfig inst;

	netConfig& netConfig::get()
	{
		return inst;
	}

	netConfig::netConfig() :
		dns_resolver(soup::make_unique<dnsOsResolver>()),
		certchain_validator(&Socket::certchain_validator_relaxed)
	{
	}
}

#endif

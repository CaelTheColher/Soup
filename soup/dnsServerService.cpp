#include "dnsServerService.hpp"

#if !SOUP_WASM

#include "dnsHeader.hpp"
#include "dnsQuestion.hpp"
#include "dnsResource.hpp"
#include "Socket.hpp"
#include "string.hpp"
#include "UniquePtr.hpp"

namespace soup
{
	dnsServerService::dnsServerService(on_query_t on_query)
		: ServerServiceUdp([](Socket& s, SocketAddr&& addr, std::string&& data, ServerServiceUdp& srv)
		{
			reinterpret_cast<dnsServerService&>(srv).handle(s, std::move(addr), std::move(data));
		}), on_query(on_query)
	{
	}

	void dnsServerService::handle(Socket& s, SocketAddr&& addr, std::string&& data)
	{
		StringReader sr(std::move(data), false);

		dnsHeader dh;
		dh.read(sr);

		if (dh.isResponse())
		{
			return;
		}

		if (dh.qdcount != 1)
		{
			return;
		}

		dnsQuestion dq;
		dq.read(sr);

		if (dq.qclass != DNS_IN)
		{
			return;
		}

		auto qname = string::join(dq.name.name, '.');

		std::vector<UniquePtr<dnsRecord>> rrs = on_query(qname, addr, (dnsType)dq.qtype);

		// TODO: Recursively resolve if A or AAAA query matches CNAME

		dh.setIsResponse(true);
		dh.bitfield1 |= (1 << 2); // AA
		dh.bitfield2 = 0; // RA = 0, Z = 0, RCODE = OK

		// Count num. answers
		dh.ancount = 0;
		for (const auto& rr : rrs)
		{
			if (rr->type == dq.qtype
				|| dq.qtype == DNS_ALL
				)
			{
				++dh.ancount;
			}
		}

		StringWriter sw(BIG_ENDIAN);
		dh.write(sw);
		dq.write(sw);

		for (const auto& rr : rrs)
		{
			if (rr->type != dq.qtype
				&& dq.qtype != DNS_ALL
				)
			{
				continue;
			}
			dnsResource dr{};
			if (rr->name == qname)
			{
				dr.name.ptr = 12; // point to name in dnsQuestion
			}
			else
			{
				dr.name.name = string::explode(rr->name, '.'); // could be more efficient for subdomains
			}
			dr.rtype = rr->type;
			dr.rclass = DNS_IN;
			dr.ttl = rr->ttl;
			dr.rdata = rr->toRdata();
			dr.write(sw);
		}
		s.udpServerSend(addr, sw.data);
	}
}

#endif

#pragma once

#include "netMeshMsgType.hpp"
#include "rsa.hpp"

namespace soup
{
	/*
	 * Soup Mesh Network is a simple way to remotely manage your servers.
	 * It is currently compatible with 'soup dnsserver', which will detect if the machine is configured to use the mesh network, and then listen for administrative commands.
	 *
	 * To set up a server with the mesh network, run 'soup mesh link' on that machine. Then, run the provided command on your personal computer.
	 * This will allow you to manage the mesh network without becoming a part of it.
	 */
	struct netMesh
	{
		[[nodiscard]] static bool isEnabled();

		struct Peer
		{
			uint32_t n_hash;
			uint32_t ip;
			Bigint n;

			[[nodiscard]] bool isRepresentedByIp(uint32_t ip) const noexcept
			{
				return this->ip == 0
					|| this->ip == ip
					;
			}

			[[nodiscard]] RsaPublicKey getPublicKey() const
			{
				return RsaPublicKey(n);
			}

			void sendAppMessage(Socket& s, netMeshMsgType msg_type, std::string&& data) const;
		};

		struct MyConfig
		{
			RsaKeypair kp;
			std::vector<Peer> peers;

			[[nodiscard]] const Peer* findPeer(uint32_t ip) const noexcept;
			[[nodiscard]] const Peer* findPeer(uint32_t n_hash, uint32_t ip) const noexcept;
		};
		[[nodiscard]] static MyConfig& getMyConfig();

		static void addPeerLocally(Bigint n, uint32_t ip = 0);
	};
}
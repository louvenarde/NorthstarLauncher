#pragma once

#include "server/serverpresence.h"
#include "engine/r2engine.h"

// Required for the very normal Microsoft API for interface enumeration
#pragma comment(lib, "iphlpapi.lib")

class LanMode
{
public:
	LanMode(bool isLanMode);

	class LanServerReporter : public ServerPresenceReporter
	{
		struct ScanningClient
		{
			// This can actually be a variety of sockets so we store the length with it
			netadr_t address;

			std::string ToString() const
			{
				std::ostringstream ipStringStream {};
				for (auto i = 0; i < sizeof(address.ip); i++)
				{
					const uint8_t byte = address.ip[i];
					const std::string characters = std::to_string(byte);
					ipStringStream << characters;

					if (i < sizeof(address.ip) - 1)
					{
						ipStringStream << ".";
					}
				}

				const std::string port = std::to_string(address.port);
				ipStringStream << ":" << port;

				return ipStringStream.str();
			}
		};

		struct Payload
		{
			uint16_t m_iPort;
			char m_sServerName[64];
			char m_sServerDesc[256];
			char m_Password[256];

			char m_MapName[32];
			char m_PlaylistName[64];
			bool m_bIsSingleplayerServer;

			uint8_t m_iPlayerCount;
			uint8_t m_iMaxPlayers;

			Payload(const struct ServerPresence* pServerPresence);
			Payload() = default;
			ServerPresence ToPresence(const SOCKADDR_INET* from, int32_t fromLength);
		};

		static_assert(sizeof(Payload) < 1200);

		friend LanMode;

	public:
		void ReportPresence(const struct ServerPresence* pServerPresence) override;
	};

	bool Enabled() { return m_bIsLanMode; }
	std::vector<ServerPresence> ScanForServers();
	void DiscoverClient(const netadr_t& adr);

private:
	bool m_bIsLanMode = false;

	_SOCKADDR_INET m_broadcastEndpoint;
	int32_t m_broadcastEndpointSize; // WSA is a strange fellow
	SOCKET m_broadcastSocket;

	bool m_bIsBroadcastSocketOK;

	uint16_t GetBroadcastPort() const;
	void SendScanPing();
	std::vector<ServerPresence> ReceivePresences(uint32_t timeout = 3000);
	void SetupBroadcastSocket();

	std::mutex m_clientListMutex {};
	std::vector<LanServerReporter::ScanningClient> m_clientsToReplyTo {};
};

extern LanMode* g_pLanMode;

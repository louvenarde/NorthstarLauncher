#include "proxy.h"
#include "lan.h"
#include "core/tier0.h"
#include "engine/r2engine.h"
#include "client/r2client.h"
#include "server/auth/serverauthentication.h"

#include <fstream>
#include <random>
#include <lmcons.h>
#include <codecvt>

#define YES true
#define NO false
///////////////////////////
//
// The current consensus at Northstar is that proxying Origin locally is an act of piracy
//	because in offline LAN mode, the game no longer requires online license verification.
//
// The day this consensus evolves, this variable can be switched to "NO" to ensure
//	the game's playability completely offline.
//
// Until then, Origin and online access will be kept mandatory for offline play.
//

constexpr bool IS_REMOVING_DRM_LOCALLY_PIRACY = YES;

//
///////////////////////////
#undef YES
#undef NO

// global vars
OriginProxy* g_originProxy;

#define PROXY_SIMPLE_SUCCESS_DECL(name)                                                                                                    \
	static OriginProxy::OriginError_t __fastcall h_##name()                                                                                \
	{                                                                                                                                      \
		return OriginProxy::OriginError_t::ORIGIN_SUCCESS;                                                                                 \
	}

#define PROXY_IMPL(name)                                                                                                                   \
	const auto o_p##name = module.GetExportedFunction(#name);                                                                              \
	HookAttach(&(PVOID&)o_p##name, (PVOID)h_##name);

PROXY_SIMPLE_SUCCESS_DECL(OriginReadEnumerationSync);
PROXY_SIMPLE_SUCCESS_DECL(OriginGrantAchievement);
PROXY_SIMPLE_SUCCESS_DECL(OriginSetPresence);
PROXY_SIMPLE_SUCCESS_DECL(OriginUpdate);
PROXY_SIMPLE_SUCCESS_DECL(OriginStartup);
PROXY_SIMPLE_SUCCESS_DECL(Tier0_GetOriginStartupResult);

static OriginProxy::OriginError_t __fastcall h_OriginGetSettingSync(int64_t inSetting, char* outSettingBuff, size_t& outBuffSize)
{
#define BOOL_STR(x) x ? "true" : "false"

	std::string setting {};
	const auto originSettings = g_originProxy->GetSettings();
	switch (inSetting)
	{
	case 0:
		setting = originSettings->Language;
		break;

	case 1:
		setting = originSettings->Environment; // Confirmed correct
		break;

	case 2:
		setting = BOOL_STR(originSettings->IsIGOAvailable);
		break;

	case 3:
		setting = BOOL_STR(originSettings->IsIGOEnabled);
		break;

	case 4:
		setting = BOOL_STR(originSettings->IsTelemetryEnabled);
		break;

	case 5:
		setting = BOOL_STR(originSettings->IsManualOffline);
		break;

	default:
		NS::log::NORTHSTAR->warn("OriginGetSettingSync({}) does not map to any known setting", inSetting);
		break;
	}

#undef BOOL_STR

	strncpy(outSettingBuff, setting.c_str(), outBuffSize);
	return OriginProxy::OriginError_t::ORIGIN_SUCCESS;
}

static OriginProxy::OriginId_t __fastcall h_OriginGetDefaultUser()
{
	return g_originProxy->GetProfile()->UserId;
}

static const char* __fastcall h_OriginGetDefaultPersona()
{
	return g_originProxy->GetProfile()->Persona;
}

static const OriginProxy::OriginError_t __fastcall h_OriginCheckOnline(bool& isOnline)
{
	isOnline = true;
	return OriginProxy::OriginError_t::ORIGIN_SUCCESS;
}

static OriginProxy::OriginError_t __fastcall h_OriginGetProfile(
	uint64_t unk1,
	uint64_t unk2,
	void(__fastcall* callbackWithProfile)(uint64_t unused, const OriginProxy::OriginGetProfileResult_t* result),
	uint64_t unk3)
{

	callbackWithProfile(0, g_originProxy->GetProfile());

	return OriginProxy::OriginError_t::ORIGIN_SUCCESS;
}

static OriginProxy::OriginError_t __fastcall h_OriginRequestAuthCode(
	int64_t userId,
	const char* clientId, // TITANFALL2-PC-SERVER
	void(__fastcall* originAuthCodeResult)(void* unk1, const char** originAuthCode),
	uint64_t unk1, // 0
	uint64_t unk2, // 3000
	uint64_t unk3 // 0
)
{
	static char originAuthCode[256] {};
	originAuthCode[0] = '\x01'; // Must not be zero
	const char* ptr = reinterpret_cast<const char*>(originAuthCode);
	originAuthCodeResult(nullptr, &ptr);
	return OriginProxy::OriginError_t::ORIGIN_SUCCESS;
}

static OriginProxy::OriginError_t __fastcall h_OriginQueryOffers(
	int64_t userId,
	const char* name, // Origin.CTY.50.0000068
	uint64_t unk1, // 1
	uint64_t unk2, // 0
	uint64_t unk3, // 0
	void(__fastcall* originQueryOffersResult)(void* unk1, uint64_t originHandle, uint64_t unk3),
	uint64_t unk4, // 0
	uint64_t unk5, // 1000
	uint64_t unk6 // 0
)
{
	originQueryOffersResult(nullptr, 0, 0); // Offer count + Offer ptr?
	return OriginProxy::OriginError_t::ORIGIN_SUCCESS;
}

ON_DLL_LOAD("tier0.dll", Tier0Proxy, (CModule module))
{
	if (g_LanMode->Enabled())
	{
		if (!IS_REMOVING_DRM_LOCALLY_PIRACY)
		{
			const auto o_pTier0_GetOriginStartupResult = module.GetExportedFunction("Tier0_GetOriginStartupResult");
			HookAttach(&(PVOID&)o_pTier0_GetOriginStartupResult, (PVOID)h_Tier0_GetOriginStartupResult);
		}
	}
}

#ifdef DEBUG_PROXY
static const char**(__fastcall* o_pGetErrorString)(int64_t errorCode) = nullptr;
static const char** __fastcall h_GetErrorString(int64_t errorCode)
{
	const char** errorInfoStr = o_pGetErrorString(errorCode);

	if (errorCode != 0)
	{
		NS::log::NORTHSTAR->warn("h_GetErrorString({}) => {}", errorCode, *errorInfoStr);
	}

	return errorInfoStr;
}

static const char*(__fastcall* o_pOriginGetErrorDescription)(int64_t errorCode) = nullptr;
static const char* __fastcall h_OriginGetErrorDescription(int64_t errorCode)
{
	const char* errorInfoStr = o_pOriginGetErrorDescription(errorCode);

	if (errorCode != 0)
	{
		NS::log::NORTHSTAR->warn("h_OriginGetErrorDescription({}) => {}", errorCode, errorInfoStr);
	}

	return errorInfoStr;
}

static const char**(__fastcall* o_pOriginGetErrorInfo)(int64_t errorCode) = nullptr;
static const char** __fastcall h_OriginGetErrorInfo(int64_t errorCode)
{
	const char** errorInfoStr = o_pOriginGetErrorInfo(errorCode);

	if (errorCode != 0)
	{
		NS::log::NORTHSTAR->warn("h_OriginGetErrorInfo({}) => {}", errorCode, *errorInfoStr);
	}

	return errorInfoStr;
}
#endif

ON_DLL_LOAD("OriginSDK.dll", OriginSDKProxy, (CModule module))
{
	if (g_LanMode->Enabled())
	{
		if (!IS_REMOVING_DRM_LOCALLY_PIRACY)
		{
			PROXY_IMPL(OriginStartup);
			PROXY_IMPL(OriginGetSettingSync);
			PROXY_IMPL(OriginGetDefaultUser);
			PROXY_IMPL(OriginGetDefaultPersona);
			PROXY_IMPL(OriginReadEnumerationSync);
			PROXY_IMPL(OriginGrantAchievement);
			PROXY_IMPL(OriginCheckOnline);
			PROXY_IMPL(OriginSetPresence);
			PROXY_IMPL(OriginUpdate);
			PROXY_IMPL(OriginGetProfile);
			PROXY_IMPL(OriginQueryOffers);
			PROXY_IMPL(OriginRequestAuthCode);
		}

#ifdef DEBUG_PROXY
		o_pOriginGetErrorInfo = module.GetExportedFunction("OriginGetErrorInfo").RCast<decltype(o_pOriginGetErrorInfo)>();
		HookAttach(&(PVOID&)o_pOriginGetErrorInfo, (PVOID)h_OriginGetErrorInfo);

		o_pOriginGetErrorDescription =
			module.GetExportedFunction("OriginGetErrorDescription").RCast<decltype(o_pOriginGetErrorDescription)>();
		HookAttach(&(PVOID&)o_pOriginGetErrorDescription, (PVOID)h_OriginGetErrorDescription);

		o_pGetErrorString = module.Offset(0x190D4).RCast<decltype(o_pGetErrorString)>();
		HookAttach(&(PVOID&)o_pGetErrorString, (PVOID)h_GetErrorString);
#endif
	}
}

#ifdef DEBUG_PROXY
static const int __stdcall h_send(SOCKET socket, const char* buf, int len, int flags)
{
	const auto result = send(socket, buf, len, flags);
	NS::log::NORTHSTAR->warn("WSA::h_send({:x}, [buff], {}, {:x}) => {:", socket, /*buf,*/ len, flags, result);

	return result;
}

static const int __stdcall h_sendto(SOCKET socket, const char* buf, int len, int flags, const sockaddr* to, int tolen)
{
	const auto result = sendto(socket, buf, len, flags, to, tolen);
	NS::log::NORTHSTAR->warn(
		"WSA::h_send({:x}, [buff], {}, {:x}, {:x}, {}) => {}", socket, /*buf,*/ len, flags, reinterpret_cast<uint64_t>(to), tolen, result);

	return result;
}
#endif

ON_DLL_LOAD("engine.dll", EngineProxy, (CModule module))
{
	if (g_LanMode->Enabled())
	{
		g_originProxy = new OriginProxy;

		// "Return NULLPTR" on a function that is a prerequisite to all HTTP CURL requests
		module.Offset(0x16FE50).Patch({0x33, 0xC0, 0xC3});

#ifdef DEBUG_PROXY
		// Override WSA to log every request in debug mode
		DWORD oldProtect {};
		auto addr = module.Offset((0x7FF88570EAC0 - 0x7FF885150000)).RCast<uint64_t*>(); //

		VirtualProtect(addr, sizeof(uint64_t), PAGE_EXECUTE_READWRITE, &oldProtect);
		*addr = reinterpret_cast<uint64_t>(&h_send);
		VirtualProtect(addr, sizeof(uint64_t), oldProtect, &oldProtect);

		addr = module.Offset((0x7FF88570EAD8 - 0x7FF885150000)).RCast<uint64_t*>();
		VirtualProtect(addr, sizeof(uint64_t), PAGE_EXECUTE_READWRITE, &oldProtect);
		*addr = reinterpret_cast<uint64_t>(&h_sendto);
		VirtualProtect(addr, sizeof(uint64_t), oldProtect, &oldProtect);

		// Extensive net task logging, not sure how to turn it on "the normal way"
		module.Offset(0x2601BC).NOP(2);
		module.Offset(0x278F9E).NOP(6);
		module.Offset(0x26011F).NOP(2);
		module.Offset(0x2634E9).NOP(2);
		module.Offset(0x263BD1).NOP(2);
		module.Offset(0x2667CB).NOP(2);
#endif
	}
}

OriginProxy::OriginProxy()
{
	CModule engineModule("engine.dll");
	this->originLastErrorPtr = engineModule.Offset(0x13978264).RCast<const OriginProxy::OriginError_t*>();
	this->wsaLastError = engineModule.Offset(0x13FA2DD0).RCast<uint32_t*>();

	const char* nameFromCommandLine {};
	if (CommandLine()->CheckParm("-playername", &nameFromCommandLine))
	{
		strncpy_s(persona, ARRAYSIZE(persona), nameFromCommandLine, strlen(nameFromCommandLine));
	}
	else
	{
		// Windows-centric username, is there another way?
		{
			CHAR username[UNLEN + 1];
			DWORD unamelen = sizeof(username) / sizeof(*username);
			bool success = GetUserNameA(username, &unamelen);

			if (success)
			{
				strncpy_s(persona, ARRAYSIZE(persona), reinterpret_cast<const char*>(username), unamelen);
			}
		}
	}

	strncpy(country, "France", ARRAYSIZE(country));

	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<uint64_t> dis;

	userId = dis(gen);

	this->originProfile.Persona = persona;
	this->originProfile.Country = country;
	this->originProfile.AvatarId = avatarId;
	this->originProfile.UserId = userId;
	this->originProfile.PersonaId = userId; // What is persona ID?

	this->originSettings.Environment = "";
	this->originSettings.IsIGOAvailable = false;
	this->originSettings.IsIGOEnabled = false;
	this->originSettings.IsManualOffline = false;
	this->originSettings.IsTelemetryEnabled = true;

	NS::log::NORTHSTAR->info("Initialized Origin Proxy with persona name [{}] and UID [{}]", persona, userId);

	CModule tier0Module("tier0.dll");
	static const char*(__fastcall * o_pDetectLanguage)() {};
	o_pDetectLanguage = tier0Module.GetExportedFunction("DetectLanguage").RCast<decltype(o_pDetectLanguage)>();
	if (o_pDetectLanguage)
	{
		this->originSettings.Language = o_pDetectLanguage();
	}
}

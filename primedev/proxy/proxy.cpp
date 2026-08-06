#include "proxy.h"
#include "lan.h"
#include <random>

// No DEBUG or _DEBUG macro in cmake?
#define DEBUG_PROXY

// global vars
OriginProxy* g_originProxy;

#define PROXY_SIMPLE_SUCCESS_DECL(name)                                                                                                    \
	static OriginProxy::OriginErrorT(__fastcall* o_p##name##)() = nullptr;                                                                 \
	static OriginProxy::OriginErrorT __fastcall h_##name##()                                                                               \
	{                                                                                                                                      \
		return OriginProxy::OriginErrorT::ORIGIN_SUCCESS;                                                                                  \
	}

#define PROXY_SIMPLE_SUCCESS_IMPL(name)                                                                                                    \
	o_p##name## = module.GetExportedFunction(#name).RCast<decltype(o_p##name##)>();                                                        \
	HookAttach(&(PVOID&)o_p##name##, (PVOID)h_##name##);


PROXY_SIMPLE_SUCCESS_DECL(OriginGrantAchievement);
PROXY_SIMPLE_SUCCESS_DECL(OriginSetPresence);
PROXY_SIMPLE_SUCCESS_DECL(OriginUpdate);

static OriginProxy::OriginErrorT(__fastcall* o_pTier0_GetOriginStartupResult)() = nullptr;
static OriginProxy::OriginErrorT __fastcall h_Tier0_GetOriginStartupResult()
{
	return OriginProxy::OriginErrorT::ORIGIN_SUCCESS;
}

static OriginProxy::OriginErrorT(__fastcall* o_pOriginGetSettingSync)(int64_t inSetting, OUT void* outSetting) = nullptr;
static OriginProxy::OriginErrorT __fastcall h_OriginGetSettingSync(int64_t inSetting, OUT void* outSetting)
{
	NS::log::NORTHSTAR->warn("h_OriginGetSettingSync({}) => {}", inSetting, OriginProxy::OriginErrorT::ORIGIN_SUCCESS);

	return OriginProxy::OriginErrorT::ORIGIN_SUCCESS;
}

static const char*(__fastcall* o_pOriginGetErrorDescription)(int64_t errorCode) = nullptr;
static const char* __fastcall h_OriginGetErrorDescription(int64_t errorCode)
{
	const char* errorInfoStr = o_pOriginGetErrorDescription(errorCode);

	NS::log::NORTHSTAR->warn("h_OriginGetErrorDescription({}) => {}", errorCode, errorInfoStr);

	if (errorCode != 0)
	{
		__debugbreak();
	}

	return errorInfoStr;
}

static const char**(__fastcall* o_pOriginGetErrorInfo)(int64_t errorCode) = nullptr;
static const char** __fastcall h_OriginGetErrorInfo(int64_t errorCode)
{
	const char** errorInfoStr = o_pOriginGetErrorInfo(errorCode);

	NS::log::NORTHSTAR->warn("h_OriginGetErrorInfo({}) => {}", errorCode, *errorInfoStr);

	if (errorCode != 0)
	{
		__debugbreak();
	}

	return errorInfoStr;
}

static OriginProxy::OriginErrorT(__fastcall* o_pOriginStartup)(unsigned int a1, unsigned __int16 a2, void* a3, OUT void* outResponse) = nullptr;
static OriginProxy::OriginErrorT __fastcall h_OriginStartup(unsigned int a1, unsigned __int16 a2, void* a3, OUT void* outResponse)
{
	return OriginProxy::OriginErrorT::ORIGIN_SUCCESS;
}


static OriginProxy::OriginId_t(__fastcall* o_pOriginGetDefaultUser)() = nullptr;
static OriginProxy::OriginId_t __fastcall h_OriginGetDefaultUser()
{
	return g_originProxy->GetProfile()->UserId;
}

static const char*(__fastcall* o_pOriginGetDefaultPersona)() = nullptr;
static const char* __fastcall h_OriginGetDefaultPersona()
{
	return g_originProxy->GetProfile()->Persona;
}

static const OriginProxy::OriginErrorT(__fastcall* o_pOriginCheckOnline)(bool& isOnline) = nullptr;
static const OriginProxy::OriginErrorT __fastcall h_OriginCheckOnline(bool& isOnline)
{
	isOnline = true;
	return OriginProxy::OriginErrorT::ORIGIN_SUCCESS;
}
static OriginProxy::OriginId_t(__fastcall* o_pOriginGetProfile)() = nullptr;
static OriginProxy::OriginErrorT __fastcall h_OriginGetProfile(
	uint64_t unk1,
	uint64_t unk2,
	void(__fastcall* callbackWithProfile)(uint64_t unused, const OriginProxy::OriginGetProfileResult_t* result),
	uint64_t unk3)
{

	callbackWithProfile(0, g_originProxy->GetProfile());

	return OriginProxy::OriginErrorT::ORIGIN_SUCCESS;
}

#ifdef DEBUG_PROXY
static const char**(__fastcall* o_pGetErrorString)(int64_t errorCode) = nullptr;
static const char** __fastcall h_GetErrorString(int64_t errorCode)
{
	const char** errorInfoStr = o_pGetErrorString(errorCode);

	NS::log::NORTHSTAR->warn("h_GetErrorString({}) => {}", errorCode, *errorInfoStr);

	if (errorCode != 0)
	{
		__debugbreak();
	}

	return errorInfoStr;
}
#endif

ON_DLL_LOAD("tier0.dll", Tier0Proxy, (CModule module))
{
	if (g_LanMode->Enabled())
	{
		o_pTier0_GetOriginStartupResult =
			module.GetExportedFunction("Tier0_GetOriginStartupResult").RCast<decltype(o_pTier0_GetOriginStartupResult)>();
		HookAttach(&(PVOID&)o_pTier0_GetOriginStartupResult, (PVOID)h_Tier0_GetOriginStartupResult);
	}
}

ON_DLL_LOAD("OriginSDK.dll", OriginSDKProxy, (CModule module))
{
	if (g_LanMode->Enabled())
	{
		o_pOriginGetSettingSync = module.GetExportedFunction("OriginGetSettingSync").RCast<decltype(o_pOriginGetSettingSync)>();
		HookAttach(&(PVOID&)o_pOriginGetSettingSync, (PVOID)h_OriginGetSettingSync);

		o_pOriginGetErrorInfo = module.GetExportedFunction("OriginGetErrorInfo").RCast<decltype(o_pOriginGetErrorInfo)>();
		HookAttach(&(PVOID&)o_pOriginGetErrorInfo, (PVOID)h_OriginGetErrorInfo);

		o_pOriginGetErrorDescription =
			module.GetExportedFunction("OriginGetErrorDescription").RCast<decltype(o_pOriginGetErrorDescription)>();
		HookAttach(&(PVOID&)o_pOriginGetErrorDescription, (PVOID)h_OriginGetErrorDescription);

		o_pOriginStartup = module.GetExportedFunction("OriginStartup").RCast<decltype(o_pOriginStartup)>();
		HookAttach(&(PVOID&)o_pOriginStartup, (PVOID)h_OriginStartup);

		o_pOriginGetDefaultUser = module.GetExportedFunction("OriginGetDefaultUser").RCast<decltype(o_pOriginGetDefaultUser)>();
		HookAttach(&(PVOID&)o_pOriginGetDefaultUser, (PVOID)h_OriginGetDefaultUser);

		o_pOriginGetDefaultPersona = module.GetExportedFunction("OriginGetDefaultPersona").RCast<decltype(o_pOriginGetDefaultPersona)>();
		HookAttach(&(PVOID&)o_pOriginGetDefaultPersona, (PVOID)h_OriginGetDefaultPersona);

		PROXY_SIMPLE_SUCCESS_IMPL(OriginGrantAchievement);
		PROXY_SIMPLE_SUCCESS_IMPL(OriginCheckOnline);
		PROXY_SIMPLE_SUCCESS_IMPL(OriginSetPresence);
		PROXY_SIMPLE_SUCCESS_IMPL(OriginUpdate);


		o_pOriginGetProfile = module.GetExportedFunction("OriginGetProfile").RCast<decltype(o_pOriginGetProfile)>();
		HookAttach(&(PVOID&)o_pOriginGetProfile, (PVOID)h_OriginGetProfile);

#ifdef DEBUG_PROXY
		o_pGetErrorString = module.Offset(0x190D4).RCast<decltype(o_pGetErrorString)>();
		HookAttach(&(PVOID&)o_pGetErrorString, (PVOID)h_GetErrorString);
#endif
	}
}

ON_DLL_LOAD("engine.dll", EngineProxy, (CModule module))
{
	if (g_LanMode->Enabled())
	{
		g_originProxy = new OriginProxy;
	}
}

OriginProxy::OriginProxy()
{
	CModule engineModule("engine.dll");
	this->originLastErrorPtr = engineModule.Offset(0x13978264).RCast<const OriginProxy::OriginErrorT*>();

	strcpy(persona, "Proxysona");
	strcpy(country, "France");

	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<uint64_t> dis;

	userId = dis(gen);
		
	this->originProfile.Persona = persona;
	this->originProfile.Country = country;
	this->originProfile.AvatarId = avatarId;
	this->originProfile.UserId = userId;
}

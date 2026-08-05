#include "proxy.h"
#include "lan.h"

// No DEBUG or _DEBUG macro in cmake?
#define DEBUG_PROXY


static HRESULT(__fastcall* o_pTier0_GetOriginStartupResult)() = nullptr;
static HRESULT __fastcall h_Tier0_GetOriginStartupResult()
{
	return ERROR_SUCCESS;
}

static HRESULT(__fastcall* o_pOriginGetSettingSync)(int64_t inSetting, OUT void* outSetting) = nullptr;
static HRESULT __fastcall h_OriginGetSettingSync(int64_t inSetting, OUT void* outSetting)
{
	NS::log::NORTHSTAR->warn("h_OriginGetSettingSync({}) => {}", inSetting, ERROR_SUCCESS);

	return ERROR_SUCCESS;
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

static HRESULT(__fastcall* o_pOriginStartup)(unsigned int a1, unsigned __int16 a2, void* a3, OUT void* outResponse) = nullptr;
static HRESULT __fastcall h_OriginStartup(unsigned int a1, unsigned __int16 a2, void* a3, OUT void* outResponse)
{
	NS::log::NORTHSTAR->warn("h_OriginStartup({}, {})", a1, a2);

	return ERROR_SUCCESS;
}

static HRESULT(__fastcall* o_pOriginUpdate)() = nullptr;
static HRESULT __fastcall h_OriginUpdate()
{
	return ERROR_SUCCESS;
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

		o_pOriginUpdate = module.GetExportedFunction("OriginUpdate").RCast<decltype(o_pOriginUpdate)>();
		HookAttach(&(PVOID&)o_pOriginUpdate, (PVOID)h_OriginUpdate);

#ifdef DEBUG_PROXY
		//o_pGetErrorString = module.Offset(0x190D4).RCast<decltype(o_pGetErrorString)>();
		//HookAttach(&(PVOID&)o_pGetErrorString, (PVOID)h_GetErrorString);
#endif
	}
}

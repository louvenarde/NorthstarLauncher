#include "origin.h"
#include <cstdint>

static bool(__fastcall* o_pCheckIfOriginIsInstalled)() = nullptr;
static bool __fastcall h_CheckIfOriginIsInstalled()
{
	return false; //

	if (!strstr(GetCommandLineA(), "-noOriginStartup"))
		return o_pCheckIfOriginIsInstalled();

	if (o_pCheckIfOriginIsInstalled())
	{
		NS::log::NORTHSTAR->warn(
			"Origin is NOT installed according to OriginSDK's check (HKLM\\SOFTWARE\\Wow6432Node\\Origin\\ClientPath is missing/empty).");
		NS::log::NORTHSTAR->warn("We are bypassing this check in case LSX server is remotely ran (such as in Linux in another WINE "
								 "prefix), but note that things will fail if Origin is actually not running.");
	}

	return true;
}

static uint64_t(__fastcall* o_pTryToStartOrigin)(void* a1) = nullptr;
static uint64_t __fastcall h_TryToStartOrigin(void* a1)
{
	return 0;//
	
	if (!strstr(GetCommandLineA(), "-noOriginStartup"))
		return o_pTryToStartOrigin(a1);

	if (o_pTryToStartOrigin(a1) != 0)
	{
		NS::log::NORTHSTAR->warn("Origin process has failed to start. We are ignoring this and let it fail on LSX connection attempt, "
								 "because LSX might still be up regardless, even if this failed.");
	}
	return 0;
}

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

ON_DLL_LOAD("OriginSDK.dll", OriginSDKFuncs, (CModule module))
{
	// these hooks are required for linux to work without EA app or Origin in the same wine prefix

	o_pCheckIfOriginIsInstalled = module.Offset(0xa1850).RCast<decltype(o_pCheckIfOriginIsInstalled)>();
	HookAttach(&(PVOID&)o_pCheckIfOriginIsInstalled, (PVOID)h_CheckIfOriginIsInstalled);

	o_pTryToStartOrigin = module.Offset(0xa19b0).RCast<decltype(o_pTryToStartOrigin)>();
	HookAttach(&(PVOID&)o_pTryToStartOrigin, (PVOID)h_TryToStartOrigin);

	o_pGetErrorString = module.Offset(0x190D4).RCast<decltype(o_pGetErrorString)>();
	HookAttach(&(PVOID&)o_pGetErrorString, (PVOID)h_GetErrorString);
	
}

// Proxy

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

ON_DLL_LOAD("tier0.dll", Tier0Proxy, (CModule module))
{
	o_pTier0_GetOriginStartupResult =
		module.GetExportedFunction("Tier0_GetOriginStartupResult").RCast<decltype(o_pTier0_GetOriginStartupResult)>();
	HookAttach(&(PVOID&)o_pTier0_GetOriginStartupResult, (PVOID)h_Tier0_GetOriginStartupResult);
}

ON_DLL_LOAD("OriginSDK.dll", OriginSDKProxy, (CModule module)) {

	o_pOriginGetSettingSync = module.GetExportedFunction("OriginGetSettingSync").RCast<decltype(o_pOriginGetSettingSync)>();
	HookAttach(&(PVOID&)o_pOriginGetSettingSync, (PVOID)h_OriginGetSettingSync);

	o_pOriginGetErrorInfo = module.GetExportedFunction("OriginGetErrorInfo").RCast<decltype(o_pOriginGetErrorInfo)>();
	HookAttach(&(PVOID&)o_pOriginGetErrorInfo, (PVOID)h_OriginGetErrorInfo);

	o_pOriginGetErrorDescription = module.GetExportedFunction("OriginGetErrorDescription").RCast<decltype(o_pOriginGetErrorDescription)>();
	HookAttach(&(PVOID&)o_pOriginGetErrorDescription, (PVOID)h_OriginGetErrorDescription);

	o_pOriginStartup = module.GetExportedFunction("OriginStartup").RCast<decltype(o_pOriginStartup)>();
	HookAttach(&(PVOID&)o_pOriginStartup, (PVOID)h_OriginStartup);

	o_pOriginUpdate = module.GetExportedFunction("OriginUpdate").RCast<decltype(o_pOriginUpdate)>();
	HookAttach(&(PVOID&)o_pOriginUpdate, (PVOID)h_OriginUpdate);
}

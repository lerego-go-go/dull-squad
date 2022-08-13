#include "MemoryUtils.hpp"

#include <winternl.h>

namespace ntspace
{
	typedef struct _LDR_DATA_TABLE_ENTRY_CUSTOM
	{
		PVOID Reserved1[2];
		LIST_ENTRY InMemoryOrderLinks;
		PVOID Reserved2[2];
		PVOID DllBase;
		PVOID Reserved3[1];
		ULONG64 SizeOfImage;
		UNICODE_STRING FullDllName;
		UNICODE_STRING BaseDllName;
		PVOID Reserved5[2];
#pragma warning(push)
#pragma warning(disable: 4201)
		union {
			ULONG CheckSum;
			PVOID Reserved6;
		} DUMMYUNIONNAME;
#pragma warning(pop)
		ULONG TimeDateStamp;
	} LDR_DATA_TABLE_ENTRY_CUSTOM, * PLDR_DATA_TABLE_ENTRY_CUSTOM;
}

class ModuleUtils
{
public:
	static VOID EraseHeaders(HMODULE hModule);

	static BYTE* GetFuncAddress(HMODULE hModule, const char* szFunc);

	static PBYTE PatternScan(LPCSTR Pattern);
	static PBYTE PatternScanInModule(uint64_t ModuleBase, LPCSTR Pattern);

	template <typename StrType>
	static uint64_t GetModuleBase(StrType ModuleName)
	{
		PPEB_LDR_DATA pLdrData = ((PTEB)__readgsqword(FIELD_OFFSET(NT_TIB, Self)))->ProcessEnvironmentBlock->Ldr;

		for (PLIST_ENTRY pEntry = pLdrData->InMemoryOrderModuleList.Flink; pEntry != &pLdrData->InMemoryOrderModuleList; pEntry = pEntry->Flink)
		{
			ntspace::LDR_DATA_TABLE_ENTRY_CUSTOM* pLdrEntry = CONTAINING_RECORD(pEntry, ntspace::LDR_DATA_TABLE_ENTRY_CUSTOM, InMemoryOrderLinks);

			if (!ModuleName)
				return (uint64_t)pLdrEntry[0].DllBase;

			if (CRT::StrCmpW(ModuleName, pLdrEntry->BaseDllName.Buffer, false))
				return (uint64_t)pLdrEntry->DllBase;
		}

		return uint64_t();
	}

	template <typename StrType>
	static uint32_t GetModuleSize(StrType ModuleName)
	{
		PPEB_LDR_DATA pLdrData = ((PTEB)__readgsqword(FIELD_OFFSET(NT_TIB, Self)))->ProcessEnvironmentBlock->Ldr;

		for (PLIST_ENTRY pEntry = pLdrData->InMemoryOrderModuleList.Flink; pEntry != &pLdrData->InMemoryOrderModuleList; pEntry = pEntry->Flink)
		{
			ntspace::LDR_DATA_TABLE_ENTRY_CUSTOM* pLdrEntry = CONTAINING_RECORD(pEntry, ntspace::LDR_DATA_TABLE_ENTRY_CUSTOM, InMemoryOrderLinks);

			if (!ModuleName)
				return (uint32_t)pLdrEntry[0].SizeOfImage;

			if (CRT::StrCmpW(ModuleName, pLdrEntry->BaseDllName.Buffer, false))
				return (uint32_t)pLdrEntry->SizeOfImage;
		}

		return uint32_t();
	}
};
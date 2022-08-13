#pragma once
#include "Loader.hpp"

#define ReadPTR(base, offset) (*(PVOID *)(((PBYTE)base + offset)))
#define ReadDWORD(base, offset) (*(PDWORD)(((PBYTE)base + offset)))
#define ReadBYTE(base, offset) (*(((PBYTE)base + offset)))

constexpr std::uintptr_t MinimumUserAddress = 0x0000000000010000;
constexpr std::uintptr_t MaximumUserAddress = 0x00007FFFFFFFFFFF;

class MemoryUtils
{
public:
	static bool IsValidPtr(uint64_t address)
	{
		return (address && sizeof(address)) ? true : false;
	}

	inline static std::uintptr_t ToAddress(const void* pointer)
	{
		return reinterpret_cast<std::uintptr_t>(pointer);
	}

	inline static bool IsUserAddress(std::uintptr_t address)
	{
		return (address >= MinimumUserAddress && address <= MaximumUserAddress);
	}

	inline static bool IsUserAddress(const void* pointer)
	{
		const auto address = ToAddress(pointer);
		return IsUserAddress(address);
	}

	inline static bool IsAddressValid(std::uintptr_t address)
	{
		return IsUserAddress(address);
	}

	inline static bool IsAddressValid(const void* pointer)
	{
		const auto address = ToAddress(pointer);
		return IsAddressValid(address);
	}

	template <typename t> t static Read(uint64_t const address)
	{
		if (IsValidPtr(address))
			return *reinterpret_cast<t*>(address);

		return t();
	}

	template <typename t> static void Write(uint64_t const address, t data)
	{
		*reinterpret_cast<t*>(address) = data;
	}

	static BOOL ReadBuffer(uint64_t address, LPVOID lpBuffer, SIZE_T nSize)
	{
		return CRT::MemCpy(lpBuffer, (LPVOID)address, nSize) != 0;
	}
};
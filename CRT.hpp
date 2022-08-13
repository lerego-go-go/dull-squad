#include <cstdint>

#define ToRVA(addr, size) ((uint8_t*)((uint64_t)(addr) + *(int32_t*)((uint64_t)(addr) + ((size) - sizeof(int32_t))) + (size)))
#define ToLowerChar(Char) ((Char >= 'A' && Char <= 'Z') ? (Char + 32) : Char)

class CRT
{
public:
	static char* __cdecl StrStr(const char* str, const char* strSearch);

	static int StrCmp(const char* string1, const char* string2);

	template <typename StrType, typename StrType2>
	static bool StrCmpW(StrType Str, StrType2 InStr, bool Two)
	{
		if (!Str || !InStr)
			return false;

		wchar_t c1, c2; do
		{
			c1 = *Str++; c2 = *InStr++;
			c1 = ToLowerChar(c1); c2 = ToLowerChar(c2);

			if (!c1 && (Two ? !c2 : 1))
				return true;

		} while (c1 == c2);

		return false;
	}

	template <typename StrType, typename StrType2>
	static __forceinline bool ÑStrStr(StrType Dst, StrType2 Str)
	{
		if (!Dst || !Str) 
			return false;

		StrType2 Str1 = Str;
		
		for (int i = 0; Dst[i]; i++) 
		{
			if (ToLowerChar(Dst[i]) != ToLowerChar(*Str1))
				Str1 = Str;
			else Str1++;
			
			if (!*Str1)
				return true;
		} 
		
		return false;
	}

	static char* StrCpy(char* strDestination, const char* strSource);
	static char* StrnCpy(char* dst, const char* src, unsigned int n);
	static char* StrnCat(char* s1, const char* s2, unsigned int n);
	static char* StrCat(char* dest, const char* src);

	static unsigned StrLen(const char* str);
	static unsigned StrLenW(const wchar_t* str);

	static void* MemCpy(void* dest, const void* source, unsigned int count);
	static void* MemSet(void* memptr, char val, unsigned int num);

	static bool m_isnan(float f);

	static int m_abs(int i);
	static double m_fabs(double x);

	static int m_ceil(float x);

	static int m_floorint(float x);
	static float m_floorf(float x);

	static float m_sqrtf(float x);
	static float m_expf(float x);

	static float m_sinf(float x);
	static float m_cosf(float x);

	static float m_atan(float x);
	static float m_tanf(float x);
	static float m_asinf(float x);

	static float m_powf(float x, float y);

	static float m_acosf(float x);
	static float m_atan2f(float x, float y);
};
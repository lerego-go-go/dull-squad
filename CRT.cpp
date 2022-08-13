#include "CRT.hpp"
#include <intrin.h>

#pragma warning(disable : 4530)
#pragma comment( linker, "/merge:.CRT=.rdata" )
#pragma comment( linker, "/merge:.pdata=.rdata" )
#pragma comment( linker, "/merge:.rdata=.text" )
#pragma comment( linker, "/merge:_RDATA=.text" )

#ifdef __cplusplus
extern "C" {
#endif
	int _fltused = 0x9875;
#ifdef __cplusplus
}
#endif

char* __cdecl CRT::StrStr(const char* str, const char* strSearch)
{
	char* cp = (char*)str;
	char* s1, * s2;

	if (!*strSearch)
		return((char*)str);

	for (; *cp;)
	{
		s1 = cp;
		s2 = (char*)strSearch;

		for (; *s1 && *s2 && !(*s1 - *s2);)
			s1++, s2++;

		if (!*s2)
			return(cp);

		cp++;
	}
	return(0);
}

int CRT::StrCmp(const char* string1, const char* string2)
{
	int ret = 0;

	unsigned char* p1 = (unsigned char*)string1;
	unsigned char* p2 = (unsigned char*)string2;

	for (; !(ret = *p1 - *p2) && *p2;)
		++p1, ++p2;

	if (ret < 0)
		ret = -1;
	else if (ret > 0)
		ret = 1;

	return ret;
}

char* CRT::StrCpy(char* strDestination, const char* strSource)
{
	char* copied = strDestination;

	for (; *strSource;)
		*strDestination++ = *strSource++; *strDestination = '\0';

	return copied;
}

unsigned CRT::StrLen(const char* str)
{
	int count = 0;

	if (!str)
		return 0;

	for (; *str != '\0'; ++str)
		++count;

	return count;
}

unsigned CRT::StrLenW(const wchar_t* str)
{
	int count = 0;

	if (!str)
		return 0;

	for (; *str != L'\0'; ++str)
		++count;

	return count;
}

char* CRT::StrnCpy(char* dst, const char* src, unsigned int n)
{
	if (n != 0) 
	{
		char* d = dst;
		const char* s = src;

		do 
		{
			if ((*d++ = *s++) == 0) 
			{
				while (--n != 0)
					*d++ = 0;
				break;
			}
		} 
		while (--n != 0);
	}

	return (dst);
}

char* CRT::StrnCat(char* s1, const char* s2, unsigned int n)
{
	unsigned len1 = StrLen(s1);
	unsigned len2 = StrLen(s2);

	if (len2 < n) 
	{

		StrCpy(&s1[len1], s2);
	}
	else
	{
		StrnCpy(&s1[len1], s2, n);
		s1[len1 + n] = '\0';
	}

	return s1;
}

char* CRT::StrCat(char* dest, const char* src)
{
	StrCpy(dest + StrLen(dest), src);
	return dest;
}

void* CRT::MemCpy(void* dest, const void* source, unsigned int count)
{
	unsigned int i;

	char* charSrc = (char*)source;
	char* charDest = (char*)dest;

	for (i = 0; i < count; i++)
		charDest[i] = charSrc[i];

	return dest;
}

void* CRT::MemSet(void* memptr, char val, unsigned int num)
{
	unsigned int filled;
	filled = (val << 24) + (val << 16) + (val << 8) + val;

	unsigned int chunks = num / sizeof(filled);
	char* charDest = (char*)memptr;

	unsigned int* uintDest = (unsigned int*)memptr;

	unsigned int i;

	for (i = num; i > chunks * sizeof(filled); i--)
		charDest[i - 1] = val;

	for (i = chunks; i > 0; i--)
		uintDest[i - 1] = filled;

	return memptr;
}

bool CRT::m_isnan(float f)
{
	const uint32_t u = *(uint32_t*)&f;
	return (u & 0x7F800000) == 0x7F800000 && (u & 0x7FFFFF);
}

int CRT::m_abs(int i)
{
	return i < 0 ? -i : i;
}

double CRT::m_fabs(double x)
{
	return x < 0 ? -x : x;
}

int CRT::m_ceil(float x)
{
	int i = (int)x;
	return x > (float)i ? i + 1 : i;
}

int CRT::m_floorint(float x)
{
	int result = _mm_cvtss_si32(_mm_floor_ss(_mm_setzero_ps(), _mm_set_ss(x)));
	return result;
}

float CRT::m_floorf(float x)
{
	float result = _mm_cvtss_f32(_mm_floor_ss(_mm_setzero_ps(), _mm_set_ss(x)));
	return result;
}

float CRT::m_sqrtf(float x)
{ 
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x))); 
}

float CRT::m_expf(float x)
{ 
	return _mm_cvtss_f32(_mm_exp_ps(_mm_set_ss(x))); 
}

float CRT::m_sinf(float x)
{ 
	return _mm_cvtss_f32(_mm_sin_ps(_mm_set_ss(x))); 
}

float CRT::m_cosf(float x)
{ 
	return _mm_cvtss_f32(_mm_cos_ps(_mm_set_ss(x))); 
}

float CRT::m_tanf(float x)
{ 
	return _mm_cvtss_f32(_mm_tan_ps(_mm_set_ss(x))); 
}

float CRT::m_atan(float x)
{
	return _mm_cvtss_f32(_mm_atan_ps(_mm_set_ss(x)));
}

float CRT::m_asinf(float x)
{ 
	return _mm_cvtss_f32(_mm_asin_ps(_mm_set_ss(x))); 
}

float CRT::m_powf(float x, float y)
{
	return _mm_cvtss_f32(_mm_pow_ps(_mm_set_ss(x), _mm_set_ss(y)));
}

float CRT::m_acosf(float x)
{
	return _mm_cvtss_f32(_mm_acos_ps(_mm_set_ss(x)));
}

float CRT::m_atan2f(float x, float y)
{
	return _mm_cvtss_f32(_mm_atan2_ps(_mm_set_ss(x), _mm_set_ss(y)));
}
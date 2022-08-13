#include "UEMath.hpp"

#define CAT_STR(x, y) x##y
#define CAT(x,y) CAT_STR(x, y)
#define PAD(Offset) BYTE CAT(pad_, __LINE__)[Offset]

template<class T>
struct TArray
{
	friend struct FString;

public:
	inline TArray()
	{
		Data = nullptr;
		Count = Max = 0;
	};

	inline INT Num() const
	{
		return Count;
	};

	inline T& operator[](INT i)
	{
		return Data[i];
	};

	inline BOOLEAN IsValidIndex(INT i)
	{
		return i < Num();
	}

private:
	T* Data;
	INT Count;
	INT Max;
};

struct FString : private TArray<WCHAR>
{
	FString()
	{
		Data = nullptr;
		Max = Count = 0;
	}

	FString(LPCWSTR other)
	{
		Max = Count = static_cast<INT>(CRT::StrLenW(other) + 1);

		if (Count)
			Data = const_cast<PWCHAR>(other);
	};

	inline bool IsValid()
	{
		return Data != nullptr;
	}

	inline const wchar_t* c_str()
	{
		return Data;
	}
};


class FText
{
private:
	PAD(0x28);
	wchar_t* Name;
	__int32 Length;

public:

	FText()
	{
		Name = nullptr;
	}

	inline bool IsValid()
	{
		return Name != nullptr;
	}

	inline wchar_t* c_str()
	{
		return Name;
	}
};

class UClass
{
public:
	PAD(0x40);
	UClass* SuperClass;
};

class UObject
{
public:
	PVOID VTableObject;
	DWORD ObjectFlags;
	DWORD InternalIndex;
	UClass* Class;
	PAD(0x8);
	UObject* Outer;

	inline BOOLEAN IsA(PVOID parentClass)
	{
		for (auto super = this->Class; super; super = super->SuperClass)
		{
			if (super == parentClass)
				return TRUE;
		}

		return FALSE;
	}
};

class FUObjectItem
{
public:
	UObject* Object;
	DWORD Flags;
	DWORD ClusterIndex;
	DWORD SerialNumber;
	DWORD SerialNumber2;
};

class TUObjectArray
{
public:
	FUObjectItem* Objects[4];
};

class GObjects
{
public:
	TUObjectArray* ObjectArray;
	PAD(0x14);
	DWORD ObjectCount;
};

typedef struct
{
	Vector3 Location;
	Vector3 Rotation;
	float FOV;
} FMinimalViewInfo;

struct FCameraCacheEntry 
{
	float Timestamp; // 0x00(0x04)
	char pad_4[0xC]; // 0x04(0x0c)
	FMinimalViewInfo POV; // 0x10(0x580)
};

struct APlayerState_GetPlayerName_Params
{
	FString PlayerName;
};

struct FBox
{
	Vector3  Min;
	Vector3  Max;
	unsigned char IsValid;
	unsigned char UnknownData00[0x3];
};

enum UEBone
{
	ROOT = 0,
	HEAD = 13,
	UPPER_NECK = 7,
	NECK = 6,
	RIGHT_SHOULDER = 66,
	RIGHT_ELBOW = 89,
	RIGHT_HAND = 124,
	LEFT_SHOULDER = 93,
	LEFT_ELBOW = 94,
	LEFT_HAND = 123,
	PELVIS = 2,
	STOMACH = 4,
	CHEST = 5,
	RIGHT_THIGH = 130,
	RIGHT_KNEE = 131,
	RIGHT_FOOT = 140,
	LEFT_THIGH = 125,
	LEFT_KNEE = 126,
	LEFT_FOOT = 139
};
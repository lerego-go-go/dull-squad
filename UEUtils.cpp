#include "UEUtils.hpp"

GObjects* gObjects = nullptr;

typedef __int64 (__fastcall* tFreeInternal)(__int64 buffer);
tFreeInternal FreeInternal = nullptr;

typedef FString* (__fastcall* tGetNameByIndex)(int* index, FString* res);
tGetNameByIndex GetNameByIndex = nullptr;

typedef char (__fastcall* tProjectWorldLocationToScreen)(PVOID PlayerController, Vector3 WorldLocation, Vector2* ScreenLocation);
tProjectWorldLocationToScreen ProjectWorldLocationToScreen = nullptr;

typedef Matrix4* (__fastcall* tGetBoneMatrix)(PVOID SkeletMesh, Matrix4* BoneMatrix, int BoneId);
tGetBoneMatrix GetBoneMatrix = nullptr;

typedef BOOL (__fastcall* tLineOfSightTo)(PVOID PlayerController, PVOID Actor, Vector3* ViewPoint, bool AlternateChecks);
tLineOfSightTo LineOfSightTo = nullptr;

typedef VOID (__fastcall* tSetShowMouseCursor)(PVOID PlayerController, unsigned __int8 bShow);
tSetShowMouseCursor SetShowMouseCursor = nullptr;

typedef __int64 (__fastcall* tGetActorBounds)(PVOID Actor, bool bOnlyCollidingComponents, Vector3* Origin, Vector3* BoxExtent, bool bIncludeFromChildActors);
tGetActorBounds GetActorBounds = nullptr;

typedef uint32_t (__fastcall* tGetSoldierTeam)(PVOID Actor);
tGetSoldierTeam GetSoldierTeam = nullptr;

VOID UEUtils::Log(LPCSTR text, ...)
{
	LI_FN(OutputDebugStringA).forwarded_safe_cached()(text);
}

VOID UEUtils::Free(uintptr_t buffer)
{
	FreeInternal(buffer);
}

void FixName(char* name)
{
	for (int i = 0; name[i] != '\0'; i++)
	{
		if (name[i] == '_')
		{
			if (name[i + 1] == '0' ||
				name[i + 1] == '1' ||
				name[i + 1] == '2' ||
				name[i + 1] == '3' ||
				name[i + 1] == '4' ||
				name[i + 1] == '5' ||
				name[i + 1] == '6' ||
				name[i + 1] == '7' ||
				name[i + 1] == '8' ||
				name[i + 1] == '9')
				name[i] = '\0';
		}
	}

	return;
}

string WstringToAnsi(const wstring& input, DWORD locale = 1251)
{
	char buf[8192] = { 0 };
	LI_FN(WideCharToMultiByte).forwarded_safe_cached()(locale, 0, input.c_str(), (int)input.length(), buf, ARRAYSIZE(buf), nullptr, nullptr);
	return buf;
}

static void* no_memcpy(void* destination, const void* source, std::size_t size)
{
	auto data_source = static_cast<const std::uint8_t*>(source);
	auto data_destination = static_cast<std::uint8_t*>(destination);

	__movsb(data_destination, data_source, size);
	return static_cast<void*>(data_destination);
}

string GetObjectNameEx(UObject* Object)
{
	if (Object == NULL)
		return "";

	int index = *(int*)(reinterpret_cast<uint64_t>(Object) + 0x18);

	FString fObjectName;
	GetNameByIndex(&index, &fObjectName);

	if (!fObjectName.IsValid())
		return "";

	if (fObjectName.c_str() == NULL)
		return "";

	wstring objectNameW = wstring(fObjectName.c_str());
	string objectName = WstringToAnsi(objectNameW);

	if (fObjectName.c_str() != NULL)
		UEUtils::Free((uintptr_t)fObjectName.c_str());

	if (objectName.empty())
		return "";

	char name[1024];
	no_memcpy(name, objectName.c_str(), 1024);

	FixName(name);

	objectNameW.clear();

	return string(name);
}

string GetObjectNameFirst(UObject* Object)
{
	if (Object == NULL)
		return "";

	int index = *(int*)(reinterpret_cast<uint64_t>(Object) + 0x18);

	FString fObjectName;
	GetNameByIndex(&index, &fObjectName);

	if (!fObjectName.IsValid())
		return "";

	if (fObjectName.c_str() == NULL)
		return "";

	wstring objectNameW = wstring(fObjectName.c_str());
	string objectName = WstringToAnsi(objectNameW);

	if (fObjectName.c_str() != NULL)
		UEUtils::Free((uintptr_t)fObjectName.c_str());

	if (objectName.empty())
		return "";

	objectNameW.clear();

	return string(objectName.c_str());
}

string UEUtils::GetObjectFirstName(UObject* object)
{
	auto internalName = GetObjectNameFirst(object);

	if (internalName.empty())
		return "";

	return internalName;
}

string GetObjectNameF(UObject* object)
{
	if (object == NULL)
		return string("");

	auto objectName = string("");

	for (auto i = 0; object; object = object->Outer, ++i)
	{
		auto internalName = GetObjectNameEx(object);

		if (internalName.empty())
			break;

		objectName = internalName + std::string(i > 0 ? "." : "") + objectName;
	}

	return objectName;
}

struct UKismetSystemLibrary_GetObjectName_Params
{
	UObject* Object;
	FString ReturnValue;
};

string KiGetObjectName(UObject* Object, bool bFix)
{
	UKismetSystemLibrary_GetObjectName_Params Params;

	Params.Object = Object;

	auto KismetGetObjectName = reinterpret_cast<UObject*>(UEOffsets::Engine::KismetSystem::GetObjectName);

	UEUtils::ProcessEvent(reinterpret_cast<UObject*>(KismetGetObjectName->Class), KismetGetObjectName, &Params);

	FString fObjectName = Params.ReturnValue;

	if (!fObjectName.IsValid())
		return "";

	wstring objectNameW = wstring(fObjectName.c_str());
	string objectName = WstringToAnsi(objectNameW);

	UEUtils::Free((uintptr_t)fObjectName.c_str());

	if (bFix)
	{
		char name[1024];
		no_memcpy(name, objectName.c_str(), 1024);

		FixName(name);

		objectNameW.clear();

		return string(name);
	}

	objectNameW.clear();

	return objectName;
}

string UEUtils::GetObjectName(UObject* object)
{
	if (object == NULL)
		return string("");

	auto objectName = string("");

	for (auto i = 0; object; object = object->Outer, ++i)
	{
		auto internalName = KiGetObjectName(object, true);

		if (internalName.empty())
			break;

		auto prefix = string(i > 0 ? "." : "");

		objectName = internalName.append(prefix).append(objectName);
	}

	return objectName;
}

PVOID FindObjectEx(LPCSTR lpObjectName)
{
	for (auto objectArray : gObjects->ObjectArray->Objects)
	{
		auto objectItem = objectArray;

		for (DWORD i = 0; i < 0x10000 && objectItem->Object; ++i, ++objectItem)
		{
			auto uObject = objectItem->Object;

			if (!uObject)
				continue;

			auto objectName = GetObjectNameF(uObject);

			if (objectName.empty())
				continue;

			if (!objectName.c_str())
				continue;

			if (CRT::StrStr(objectName.c_str(), lpObjectName))
			{
				auto currentSize = CRT::StrLen(lpObjectName);
				auto targetSize = CRT::StrLen(objectName.c_str());

				if (currentSize == targetSize)
				{
					objectName.clear();
					return uObject;
				}
			}

			objectName.clear();
		}
	}

	return 0;
}

PVOID UEUtils::FindObject(LPCSTR lpObjectName)
{
	for (auto objectArray : gObjects->ObjectArray->Objects)
	{
		auto objectItem = objectArray;

		for (DWORD i = 0; i < 0x10000 && objectItem->Object; ++i, ++objectItem)
		{
			auto uObject = objectItem->Object;

			if (!uObject)
				continue;

			auto objectName = GetObjectName(uObject);

			if (objectName.empty())
				continue;

			if (!objectName.c_str())
				continue;

			if (CRT::StrStr(objectName.c_str(), lpObjectName))
			{
				auto currentSize = CRT::StrLen(lpObjectName);
				auto targetSize = CRT::StrLen(objectName.c_str());

				if (currentSize == targetSize)
				{
					objectName.clear();
					return uObject;
				}
			}

			objectName.clear();
		}
	}

	return 0;
}

VOID UEUtils::ProcessEvent(UObject* Class, UObject* Function, PVOID Params)
{
	auto vtable = *reinterpret_cast<void***>(Class);
	reinterpret_cast<void(*)(void*, void*, void*)>(vtable[0x42])(Class, Function, Params);
}

BOOL UEUtils::WorldToScreen(PVOID PlayerController, Vector3 WorldLocation, Vector2* ScreenLocation)
{
	return ProjectWorldLocationToScreen(PlayerController, WorldLocation, ScreenLocation);
}

FBox UEUtils::GetBoundingBox(PVOID Actor)
{
	Vector3 Origin = Vector3();
	Vector3 BoxExtend = Vector3();

	GetActorBounds(Actor, true, &Origin, &BoxExtend, false);

	if (Origin.Invalid() && BoxExtend.Invalid())
	{
		FBox BoundingBox;

		BoundingBox.IsValid = 0;

		return BoundingBox;
	}

	FBox BoundingBox;

	BoundingBox.IsValid = 1;

	BoundingBox.Min = Origin - BoxExtend;
	BoundingBox.Max = Origin + BoxExtend;

	return BoundingBox;
}

Vector3 UEUtils::GetBonePosition(PVOID SkeletMesh, int BoneId)
{
	Vector3 BonePosition = Vector3();

	Matrix4 BoneMatrix;
	if (!(GetBoneMatrix(SkeletMesh, &BoneMatrix, BoneId)))
		return BonePosition;

	BonePosition.x = BoneMatrix._41;
	BonePosition.y = BoneMatrix._42;
	BonePosition.z = BoneMatrix._43;

	return BonePosition;
}

Vector3 UEUtils::GetPlayerPosition(PVOID Actor)
{
	auto PlayerRoot = ReadPTR(Actor, UEOffsets::Engine::Actor::RootComponent);

	if (!PlayerRoot)
		return Vector3();

	return *reinterpret_cast<Vector3*>(reinterpret_cast<PBYTE>(PlayerRoot) + UEOffsets::Engine::SceneComponent::RelativeLocation);
}

Vector3 UEUtils::GetPlayerVelocity(PVOID Actor)
{
	auto PlayerRoot = ReadPTR(Actor, UEOffsets::Engine::Actor::RootComponent);

	if (!PlayerRoot)
		return Vector3();

	return *reinterpret_cast<Vector3*>(reinterpret_cast<PBYTE>(PlayerRoot) + UEOffsets::Engine::SceneComponent::ComponentVelocity);
}

FString UEUtils::GetPlayerName(PVOID PlayerState)
{
	APlayerState_GetPlayerName_Params Params;

	UEUtils::ProcessEvent(reinterpret_cast<UObject*>(PlayerState), reinterpret_cast<UObject*>(UEOffsets::Engine::PlayerState::GetPlayerName), &Params);

	FString PlayerName = FString(xor (L"none"));
	PlayerName = Params.PlayerName;

	return PlayerName;
}

float UEUtils::GetPlayerDistance(Vector3 ActorLocation, Vector3 PlayerLocation)
{
	Vector3 Difference = PlayerLocation - ActorLocation;

	return Difference.Length();
}

BOOLEAN UEUtils::IsVisible(PVOID PlayerController, PVOID Actor)
{
	Vector3 tmp;
	return LineOfSightTo(PlayerController, Actor, &tmp, false);
}

bool UEUtils::IsDead(PVOID Actor)
{
	float Health = 0.0f;
	Health = MemoryUtils::Read<float>(reinterpret_cast<uint64_t>(Actor) + UEOffsets::Game::SQSoldier::Health);

	return Health <= 0.0f;
}

bool UEUtils::IsInTeam(PVOID Actor, PVOID LocalActor)
{
	uint32_t ActorTeam = 0, LocalTeam = 0;

	ActorTeam = GetSoldierTeam(Actor);
	LocalTeam = GetSoldierTeam(LocalActor);

	return ActorTeam == LocalTeam;
}

VOID UEUtils::SetControlRotation(PVOID PlayerController, Vector3 NewRotation)
{
	MemoryUtils::Write<Vector3>(reinterpret_cast<uint64_t>(PlayerController) + 0x2B0, NewRotation);
}

PVOID UEUtils::GetCurrentWeapon(PVOID Actor)
{
	auto InventoryComponent = ReadPTR(Actor, UEOffsets::Game::SQSoldier::InventoryComponent);
	
	if (!InventoryComponent)
		return 0;

	auto CurrentWeapon = ReadPTR(InventoryComponent, 0x168);

	if (!CurrentWeapon)
		return 0;

	return CurrentWeapon;
}

BOOL UEUtils::Init()
{
	auto address = ModuleUtils::PatternScan(xor ("48 8B 05 ? ? ? ? 48 8B 1C C8 48 85 DB 75 07"));

	if (!address)
		return FALSE;

	gObjects = reinterpret_cast<decltype(gObjects)>(ToRVA(address, 7));

	address = ModuleUtils::PatternScan(xor ("40 57 48 83 EC 40 48 C7 44 24 ? ? ? ? ? 48 89 5C 24 ? 48 8B DA 83 79"));

	if (!address)
		return FALSE;

	GetNameByIndex = (tGetNameByIndex) address;

	address = ModuleUtils::PatternScan(xor ("48 85 C9 74 2E 53 48 83 EC 20 48 8B D9 48 8B 0D ? ? ? ? 48 85 C9 75 0C"));

	if (!address)
		return FALSE;

	FreeInternal = (tFreeInternal)address;

	address = ModuleUtils::PatternScan(xor ("E8 ? ? ? ? 41 88 06 48 83 C4 30 41 5E"));

	if (!address)
		return FALSE;

	auto ProjectWorldLocationToScreenPtr = ToRVA(address, 5);

	ProjectWorldLocationToScreen = (tProjectWorldLocationToScreen)ProjectWorldLocationToScreenPtr;

	address = ModuleUtils::PatternScan(xor ("E8 ? ? ? ? 48 8B 8C 24 ? ? ? ? 0F 28 00 0F 29"));

	if (!address)
		return FALSE;

	auto GetBoneMatrixPtr = ToRVA(address, 5);

	GetBoneMatrix = (tGetBoneMatrix)GetBoneMatrixPtr;

	address = ModuleUtils::PatternScan(xor ("48 8B C4 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 C7 44 24 ? ? ? ? ? 48 89 58 20 0F 29 70 B8 0F 29 78 A8 44 0F 29 40 ? 44"));

	if (!address)
		return FALSE;

	LineOfSightTo = (tLineOfSightTo)address;

	address = ModuleUtils::PatternScan(xor ("48 89 5C 24 ? 57 48 83 EC 60 48 8B 01 80 F2 01 49 8B F8 0F 29 74 24 ?"));

	if (!address)
		return FALSE;

	GetActorBounds = (tGetActorBounds)address;

	address = ModuleUtils::PatternScan(xor ("E8 ? ? ? ? 39 86 ? ? ? ? 0F 94 C0 48 83 C4 48 5F 5E C3"));

	if (!address)
		return FALSE;

	auto GetTeamPtr = ToRVA(address, 5);

	GetSoldierTeam = (tGetSoldierTeam) GetTeamPtr;

	UEOffsets::Engine::KismetSystem::GetObjectName = FindObjectEx(xor ("/Script/Engine.KismetSystemLibrary.GetObjectName"));

	if (!UEOffsets::Engine::KismetSystem::GetObjectName)
		return FALSE;

	UEOffsets::Engine::PlayerController::WasInputKeyJustPressed = UEUtils::FindObject(xor ("/Script/Engine.PlayerController.WasInputKeyJustPressed"));

	if (!UEOffsets::Engine::PlayerController::WasInputKeyJustPressed)
		return FALSE;

	UEOffsets::Engine::PlayerController::WasInputKeyJustReleased = UEUtils::FindObject(xor ("/Script/Engine.PlayerController.WasInputKeyJustReleased"));

	if (!UEOffsets::Engine::PlayerController::WasInputKeyJustReleased)
		return FALSE;

	return TRUE;
}
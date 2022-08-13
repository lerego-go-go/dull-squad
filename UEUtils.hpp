#include "UEOffsets.hpp"

class UEUtils
{
public:
	static BOOL Init();

	static VOID Log(LPCSTR text, ...);
	static VOID Free(uintptr_t buffer);

	static string GetObjectName(UObject* object);
	static string GetObjectFirstName(UObject* object);

	static PVOID FindObject(LPCSTR lpObjectName);

	static Vector3 GetBonePosition(PVOID SkeletMesh, int BoneId);

	static Vector3 GetPlayerPosition(PVOID Actor);
	static Vector3 GetPlayerVelocity(PVOID Actor);

	static FString GetPlayerName(PVOID PlayerState);

	static float GetPlayerDistance(Vector3 ActorLocation, Vector3 PlayerLocation);

	static VOID ProcessEvent(UObject* Class, UObject* Function, PVOID Params);

	static BOOL WorldToScreen(PVOID PlayerController, Vector3 WorldLocation, Vector2* ScreenLocation);
	static FBox GetBoundingBox(PVOID Actor);

	static BOOLEAN IsVisible(PVOID PlayerController, PVOID Actor);

	static bool IsDead(PVOID Actor);
	static bool IsInTeam(PVOID Actor, PVOID LocalActor);

	static VOID SetControlRotation(PVOID PlayerController, Vector3 NewRotation);

	static PVOID GetCurrentWeapon(PVOID Actor);
};
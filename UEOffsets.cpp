#include "UEUtils.hpp"

namespace UEOffsets
{
	PVOID* uWorld = 0;

	namespace Engine
	{
		namespace World
		{
			DWORD OwningGameInstance = 0x170;
			DWORD Levels = 0x148;
			DWORD PersistentLevel = 0x30;
		}

		namespace Level
		{
			DWORD AActors = 0x98; // OwningWorld offset - 0x20
			DWORD AActorCount = 0xA0;
		}

		namespace KismetSystem
		{
			PVOID GetObjectName = 0;
			PVOID Vector_IsNAN = 0;
		}

		namespace GameplayStatics
		{
			PVOID GetAllActorsOfClass = 0;
		}

		namespace Canvas
		{
			PVOID K2_DrawText = 0;
			PVOID K2_DrawLine = 0;
			PVOID K2_StrLen = 0;
		}

		namespace AHUD
		{
			PVOID DrawRect = 0;
		}

		namespace Fonts
		{
			PVOID RobotoLight = 0;
		}

		namespace GameInstance
		{
			DWORD LocalPlayers = 0x38;
		}

		namespace Player
		{
			DWORD PlayerController = 0x30;
		}

		namespace PlayerController
		{
			DWORD AcknowledgedPawn = 0x2C8;
			DWORD MyHUD = 0x2D8;
			DWORD PlayerCameraManager = 0x2E0;
			PVOID GetInputKeyTimeDown = 0;
			PVOID WasInputKeyJustPressed = 0;
			PVOID WasInputKeyJustReleased = 0;
			PVOID GetMousePosition = 0;
			PVOID SetControlRotation = 0;
			PVOID SetIgnoreLookInput = 0;
			PVOID IsLookInputIgnored = 0;
		}

		namespace PlayerCameraManager
		{
			DWORD CameraCachePrivate = 0x1A40;
		}

		namespace Pawn
		{
			DWORD PlayerState = 0x268;
		}

		namespace Actor
		{
			DWORD RootComponent = 0x160;
		}

		namespace PlayerState
		{
			DWORD PlayerNamePrivate = 0x350;
			PVOID GetPlayerName = 0;
		}

		namespace Character
		{
			DWORD Mesh = 0x2A8;
			DWORD CharacterMovement = 0x2B0;
		}

		namespace SceneComponent
		{
			DWORD RelativeLocation = 0x144;
			DWORD ComponentVelocity = 0x168;
		}

		namespace StaticMeshComponent
		{
			DWORD StaticMesh = 0x488;
			DWORD ComponentToWorld = 0x1C0;
		}
	}

	namespace Game
	{
		namespace SQSoldier
		{
			DWORD Health = 0x1C18;
			DWORD InventoryComponent = 0x1E30;
		}
	}

	BOOL Init()
	{
		auto uWorldPtr = ModuleUtils::PatternScan(xor ("48 8B 1D ? ? ? ? 48 85 DB 74 3B 41 B0 01 33 D2 48 8B CB E8"));

		if (!uWorldPtr)
		{
			return FALSE;
		}

		uWorld = reinterpret_cast<decltype(uWorld)>(ToRVA(uWorldPtr, 7));

		Engine::PlayerController::SetControlRotation = UEUtils::FindObject(xor ("/Script/Engine.Controller.SetControlRotation"));

		if (!UEOffsets::Engine::PlayerController::SetControlRotation)
			return FALSE;

		Engine::PlayerState::GetPlayerName = UEUtils::FindObject(xor ("/Script/Engine.PlayerState.GetPlayerName"));

		if (!Engine::PlayerState::GetPlayerName)
		{
			return FALSE;
		}

		return TRUE;
	}
}
#include "UEStructs.hpp"

namespace UEOffsets
{
	extern PVOID* uWorld;

	namespace Engine
	{
		namespace World
		{
			extern DWORD OwningGameInstance;
			extern DWORD Levels;
			extern DWORD PersistentLevel;
		}

		namespace GameplayStatics
		{
			extern PVOID GetAllActorsOfClass;
		}

		namespace Level
		{
			extern DWORD AActors;
			extern DWORD AActorCount;
		}

		namespace KismetSystem
		{
			extern PVOID GetObjectName;
			extern PVOID Vector_IsNAN;
		}

		namespace Canvas
		{
			extern PVOID K2_DrawText;
			extern PVOID K2_DrawLine;
			extern PVOID K2_StrLen;
		}

		namespace AHUD
		{
			extern PVOID DrawRect;
		}

		namespace Fonts
		{
			extern PVOID RobotoLight;
		}

		namespace GameInstance
		{
			extern DWORD LocalPlayers;
		}

		namespace Player
		{
			extern DWORD PlayerController;
		}

		namespace PlayerController
		{
			extern DWORD AcknowledgedPawn;
			extern DWORD MyHUD;
			extern DWORD PlayerCameraManager;
			extern PVOID GetInputKeyTimeDown;
			extern PVOID WasInputKeyJustPressed;
			extern PVOID WasInputKeyJustReleased;
			extern PVOID GetMousePosition;
			extern PVOID SetControlRotation;
			extern PVOID SetIgnoreLookInput;
			extern PVOID IsLookInputIgnored;
		}

		namespace PlayerCameraManager
		{
			extern DWORD CameraCachePrivate;
		}

		namespace Pawn
		{
			extern DWORD PlayerState;
		}

		namespace Actor
		{
			extern DWORD RootComponent;
		}

		namespace PlayerState
		{
			extern DWORD PlayerNamePrivate;
			extern PVOID GetPlayerName;
		}

		namespace Character
		{
			extern DWORD Mesh;
			extern DWORD CharacterMovement;
		}

		namespace SceneComponent
		{
			extern DWORD RelativeLocation;
			extern DWORD ComponentVelocity;
		}

		namespace StaticMeshComponent
		{
			extern DWORD StaticMesh;
			extern DWORD ComponentToWorld;
		}
	}

	namespace Game
	{
		namespace SQSoldier
		{
			extern DWORD Health;
			extern DWORD InventoryComponent;
		}
	}

	BOOL Init();
}
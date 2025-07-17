#include "ActionRPGGameMode.h"
#include "ActionRPGCharacter.h"
#include "Character/Player/MyPlayerController.h"
#include "UObject/ConstructorHelpers.h"

AActionRPGGameMode::AActionRPGGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ActionRPG/Character/BP_PlayerCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/ActionRPG/Character/BP_PlayerController"));
	if (PlayerControllerBPClass.Succeeded())
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}

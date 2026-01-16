// Copyright Epic Games, Inc. All Rights Reserved.

#include "DawnOf10MinsGameMode.h"
#include "DawnOf10MinsPlayerController.h"
#include "DawnOf10MinsCharacter.h"
#include "UObject/ConstructorHelpers.h"

ADawnOf10MinsGameMode::ADawnOf10MinsGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ADawnOf10MinsPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}
// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceShooter_3DGameMode.h"
#include "UObject/ConstructorHelpers.h"

ASpaceShooter_3DGameMode::ASpaceShooter_3DGameMode()
{
	// set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	//if (PlayerPawnBPClass.Class != NULL)
	//{
	//	DefaultPawnClass = PlayerPawnBPClass.Class;
	//}
}

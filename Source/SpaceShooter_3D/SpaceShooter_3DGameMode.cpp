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

FShipAttribute ASpaceShooter_3DGameMode::GetAttribute(const EShipType& ShipToUse) const
{
	if (!ShipData) {
		UE_LOG(LogTemp, Error, TEXT("Invalid DataTable"));
		return FShipAttribute();
	}
	FString ShipString = UEnum::GetValueAsString(ShipToUse);
	const FShipAttribute* data = ShipData->FindRow<FShipAttribute>(FName(ShipString), TEXT("Searching Ship data from data table"));

	if (!data) {
		UE_LOG(LogTemp, Error, TEXT("Invalid DataTable Row"));
		return FShipAttribute();
	}
	return *data;
}

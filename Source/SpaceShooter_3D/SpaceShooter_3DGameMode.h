// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Character/SpaceShooter_3DCharacter.h"
#include "SpaceShooter_3DGameMode.generated.h"

class UDataTable;
struct FShipAttribute;

UCLASS(minimalapi)
class ASpaceShooter_3DGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASpaceShooter_3DGameMode();
	FShipAttribute GetAttribute(const EShipType& ShipToUse) const;

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDataTable> ShipData;
};




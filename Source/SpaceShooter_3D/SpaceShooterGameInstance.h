// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Character/SpaceShooter_3DCharacter.h"
#include "SpaceShooterGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SPACESHOOTER_3D_API USpaceShooterGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetPlayerShipType(EShipType NewType);

	UFUNCTION(BlueprintCallable)
	EShipType GetPlayerShipType() const;

protected:
	EShipType PlayerShipType = EShipType::GoldenVector;
	
};

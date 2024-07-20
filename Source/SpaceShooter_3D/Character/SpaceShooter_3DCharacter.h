// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "SpaceShooter_3DCharacter.generated.h"


class UAStarAgentComponent;
class AShipProjectile;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

USTRUCT(Blueprintable)
struct FShipArmor : public FTableRowBase{
	GENERATED_BODY();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AShipProjectile> Missle01;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Missle01_CD = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Missle01_Damage = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AShipProjectile> Missle02;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Missle02_CD = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Missle02_Damage = 1.0f;
};

UCLASS(config=Game)
class ASpaceShooter_3DCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASpaceShooter_3DCharacter();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AIAgent, meta = (AllowPrivateAccess = "true"))
	UAStarAgentComponent* AgentComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ChildACtor, meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* ShipChildActor;

};


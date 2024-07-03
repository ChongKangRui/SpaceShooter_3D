// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "SpaceShooter_3DCharacter.generated.h"


class UAStarAgentComponent;


DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ASpaceShooter_3DCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AIAgent, meta = (AllowPrivateAccess = "true"))
	UAStarAgentComponent* AgentComponent;


public:
	ASpaceShooter_3DCharacter();
	
			

};


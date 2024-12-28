// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIController_SpaceEnemy.generated.h"

/**
 * 
 */
UCLASS()
class SPACESHOOTER_3D_API AAIController_SpaceEnemy : public AAIController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetTargetAsPlayer();
	UFUNCTION(BlueprintCallable)
	void SetTargetFromTag();
	UFUNCTION(BlueprintCallable)
	bool HaveTarget() const;
	
public:
	UPROPERTY(EditAnywhere)
	FName EnemyTag;

protected:
	AActor* Target;
};

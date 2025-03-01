// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/SpaceShooter_3DCharacter.h"
#include "SpaceShooter_AI.generated.h"

class AAIController_SpaceEnemy;
/**
 * 
 */
UCLASS()
class SPACESHOOTER_3D_API ASpaceShooter_AI : public ASpaceShooter_3DCharacter
{
	GENERATED_BODY()

public:
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnTakeAnyDamageBinding(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser) override;
	virtual const FVector GetShootDirection() const override;
	virtual AActor* GetMissleTrackEnemy() const override;
private:
	TObjectPtr<AAIController_SpaceEnemy> m_AIController;
};

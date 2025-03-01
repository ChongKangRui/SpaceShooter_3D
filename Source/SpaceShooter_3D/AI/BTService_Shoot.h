// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_Shoot.generated.h"

class AAIController_SpaceEnemy;
class ASpaceShooter_3DCharacter;

UCLASS()
class SPACESHOOTER_3D_API UBTService_Shoot : public UBTService_BlackboardBase
{
	GENERATED_BODY()
public:

	UPROPERTY(EditInstanceOnly)
	float AngleToShoot = 30.0f;

	UPROPERTY(EditInstanceOnly)
	bool DisableNormalAttack = false;

	UPROPERTY(EditInstanceOnly)
	FVector2D FrequencyOfHeavyWeaponAttack = {5, 20};

	UPROPERTY(EditInstanceOnly)
	FVector2D DurationOfHeavyWeaponShooting = { 7, 15 };

	
protected:
	UBTService_Shoot();
	virtual void OnSearchStart(FBehaviorTreeSearchData& SearchData) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	void ShootHeavyWeapon();

private:
	TObjectPtr<AAIController_SpaceEnemy> m_OwnerController;
	TObjectPtr<ASpaceShooter_3DCharacter> m_OwnerCharacter;

	float m_HeavyWeaponCD;
	float m_HeavyWeaponShootDuration;

	float m_CurrentHWTimer;
	float m_CurrentHWShootDuration;
};

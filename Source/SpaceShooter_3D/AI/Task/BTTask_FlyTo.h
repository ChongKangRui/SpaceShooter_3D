// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_FlyTo.generated.h"



UENUM(BlueprintType)
enum EFlyingMode {
	RandomFlying,
	FlyToTarget,
	FlyAroundTarget
};

class UAStarAgentComponent;
class AAIController;

UCLASS()
class SPACESHOOTER_3D_API UBTTask_FlyTo : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
protected:
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	void UpdateTargetLocation();

protected:
	UPROPERTY(EditInstanceOnly);
	TEnumAsByte<EFlyingMode> FlyingMode;

	UPROPERTY(EditInstanceOnly,Category= "RandomFlying And FlyAroundTarget", meta = (EditCondition = "FlyingMode == EFlyingMode::FlyAroundTarget || FlyingMode == EFlyingMode::RandomFlying"));
	float MinimumRadius = 2500;

	UPROPERTY(EditInstanceOnly, Category = "RandomFlying And FlyAroundTarget", meta = (EditCondition = "FlyingMode == EFlyingMode::FlyAroundTarget|| FlyingMode == EFlyingMode::RandomFlying"));
	float MaximumRadius = 5000;

	UPROPERTY(EditInstanceOnly, Category = FlyAroundTarget, meta = (EditCondition = "FlyingMode == EFlyingMode::FlyAroundTarget"));
	float MinimumAngle = 45;

	UPROPERTY(EditInstanceOnly, Category = FlyAroundTarget, meta = (EditCondition = "FlyingMode == EFlyingMode::FlyAroundTarget"));
	float MaximumAngle = 90;

	UPROPERTY(EditInstanceOnly, Category = FlyToTarget, meta = (EditCondition = "FlyingMode == EFlyingMode::FlyToTarget"));
	float DistanceToKeepTrack = 150;

	UPROPERTY(EditInstanceOnly, Category = FlyToTarget, meta = (EditCondition = "FlyingMode == EFlyingMode::FlyToTarget"));
	bool RotateTowardTarget = true;

	UPROPERTY(EditInstanceOnly, Category = FlyToTarget, meta = (EditCondition = "RotateTowardTarget"));
	float RotateSpeed = 50.0f;

	UPROPERTY(EditInstanceOnly, Category = Generic);
	float AcceptableRadius = 150;

	UPROPERTY(EditInstanceOnly);
	FBlackboardKeySelector Target;


private:
	UFUNCTION()
	void OnAgentMoving(FVector NextPosition);

	UFUNCTION()
	void OnAgentStateChanged(EPathfindingStatus newStatus);

private:
	TObjectPtr<AActor> m_Target;
	TObjectPtr<UAStarAgentComponent> m_Agent;
	TObjectPtr<AAIController> m_AIController;
	float m_AgentDefaultsAcceptableRange;
	
};

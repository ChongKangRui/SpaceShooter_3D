// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AStarPathGrid.h"
#include "AStarAgentComponent.generated.h"

UENUM(Blueprintable)
enum class  EPathfindingStatus : uint8 {
	None,
	InProgress,
	Failed,
	Success


};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPACESHOOTER_3D_API UAStarAgentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAStarAgentComponent();
	
	UFUNCTION(BlueprintCallable)
	void MoveTo(FVector Goal);

	UFUNCTION(BlueprintPure)
	EPathfindingStatus GetAgentStatus() const;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	TArray<FVector> ReconstructPath(const UAStarNode* Goal, const TArray<FAStarNodeData> NodeList);
	void AgentMove(TArray<FVector>& Path);
private:	
	
	TObjectPtr<AAStarPathGrid> m_PathGrid;
	EPathfindingStatus m_AgentStatus = EPathfindingStatus::None;
	
	FTimerHandle m_PathFindingHandle;
		
};

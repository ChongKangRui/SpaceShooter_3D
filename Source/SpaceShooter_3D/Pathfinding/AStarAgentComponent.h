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

class ASpaceShooter_3DCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAIPathfindingStateChanged, EPathfindingStatus, AgentStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAgentMoving, FVector, MovingDestination);

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

	UFUNCTION(BlueprintPure)
	AAStarPathGrid* GetGridReference() const;

	UFUNCTION(BlueprintPure)
	FVector GetCurrentMovingLocation() const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PathFinding)
	float RotationSpeed = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PathFinding)
	float FlyingSpeed = 1000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PathFinding)
	float AcceptableRange = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PathFinding)
	float PenaltyWeight = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PathFinding|BezierPath")
	bool EnableBezierPath = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PathFinding|BezierPath")
	int SegmentPerPath = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PathFinding|Debug")
	bool EnablePathFindingDebug;

	UPROPERTY(BlueprintAssignable)
	FOnAIPathfindingStateChanged OnPathfindingStateChanged;

	UPROPERTY(BlueprintAssignable)
	FOnAgentMoving OnAgentMoving;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float Delta, ELevelTick type, FActorComponentTickFunction* func) override;

private:
	TArray<FVector> ReconstructPath(const FNodeRealData& Goal, const TArray<TArray<TArray<FAStarNodeData>>> NodeList);
	void AgentMove();

	void DrawDebugPath(const TArray<FVector>& path);

	void SetPathfindingState(const EPathfindingStatus& newStatus);

	TArray<FVector> GenerateBezierPath(TArray<FVector> path);

private:	
	
	TArray<FVector> m_Path;
	TObjectPtr<AAStarPathGrid> m_PathGrid;
	EPathfindingStatus m_AgentStatus = EPathfindingStatus::None;
	
	FTimerHandle m_PathFindingHandle;

	TObjectPtr<ASpaceShooter_3DCharacter> m_Agent;

	FIntVector m_PreviousLocation;
};

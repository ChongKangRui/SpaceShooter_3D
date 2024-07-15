// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AStarNode.generated.h"

UENUM(Blueprintable)
enum ENodeStatus : uint8 {
	AllowToPass,
	InvalidPath,
	ShipOccupied


};


UCLASS()
class SPACESHOOTER_3D_API UAStarNode : public USceneComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UAStarNode();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
	int cost = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Pathfinding")
	TArray<UAStarNode*> Neightbours;

	FIntVector Point;
	ENodeStatus Status = ENodeStatus::AllowToPass;

	bool CheckIsNodeOccupied() const;
	bool CheckIsNodeInvalid() const;

};

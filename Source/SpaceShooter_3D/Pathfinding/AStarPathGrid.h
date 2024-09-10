// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AStarNode.h"
#include "AStarPathGrid.generated.h"


//USTRUCT(Blueprintable)
//struct FAStarNodeStruct {
//	GENERATED_BODY()
//
//public:
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
//	FVector Position;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
//	bool InvalidPath = false;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
//	int cost = 1;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
//	TArray<FVector> Neightbours;
//
//	FAStarNodeStruct() : Position(FVector(0.0f)), InvalidPath(false), cost(1) {
//
//	}
//	FAStarNodeStruct(FVector p) : Position(p), InvalidPath(false), cost(1) {
//
//	}
//
//};

UENUM(Blueprintable)
enum ENodeStatus : uint8 {
	AllowToPass,
	InvalidPath,
	ShipOccupied
};

USTRUCT()
struct FNodeRealData {
	GENERATED_BODY()
public:
	FVector Location = FVector::Zero();
	ENodeStatus Status = ENodeStatus::InvalidPath;

	TArray<FIntVector> Neightbours;

	bool CheckIsNodeOccupied() const {
		return Status == ENodeStatus::ShipOccupied;
	};
	bool CheckIsNodeInvalid() const {
		return Status == ENodeStatus::InvalidPath;
	};


	FNodeRealData() = default;
	// Constructor that takes an FVector parameter
	explicit FNodeRealData(FVector loc) : Location(loc) {}

	bool operator==(const FNodeRealData& Other) const
	{
		return Location == Other.Location && Status == Other.Status;
	}

	bool operator==(const FNodeRealData* Other) const
	{
		return this == Other;
	}

private:

};


USTRUCT()
struct FAStarNodeData {
		GENERATED_BODY()
	
	public:
		float gCost;
		float hCost;

		//UAStarNode* Node;
		//UAStarNode* CameFrom;

		FNodeRealData* Node;
		FNodeRealData* CameFrom;
		
		FAStarNodeData() : gCost(FLT_MAX), hCost(FLT_MAX), Node(nullptr), CameFrom(nullptr){
		}

		FAStarNodeData(FNodeRealData& n) : gCost(FLT_MAX), hCost(FLT_MAX), CameFrom(nullptr) {
			Node = &n;
		}

		bool operator==(const FAStarNodeData& Other) const
		{
			return Node == Other.Node;
		}

		bool operator==(const FNodeRealData& node) const {
			return Node == node;
		}

		bool operator<(const FAStarNodeData& Other) const {
			return GetFCost() < Other.GetFCost();
		}

		

		const bool IsValidNode() const {
			return Node && !Node->CheckIsNodeInvalid();
		}
		
		const FVector GetLocation() const {
			if (IsValidNode()) {
				return Node->Location;
			}
			return FVector::Zero();
		}

		TArray<FIntVector>& GetNeighbour() const {
			static TArray<FIntVector> EmptyIndexArray;
			if (Node)
				return Node->Neightbours;
			else
				return EmptyIndexArray;
		}

		const float GetFCost() const {
			return gCost + hCost;
		}

};

UCLASS()
class SPACESHOOTER_3D_API AAStarPathGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	AAStarPathGrid();

	UFUNCTION(CallInEditor, Category = "Setting")
	void RefreshPathFinding();

	UFUNCTION(BlueprintPure)
	FVector GetRandomLocationWithinRange(const FVector Center, const float MinimumRange, const float MaximumRange);

	FNodeRealData& GetClosestNode(FVector Position);
	FNodeRealData& GetNode(FIntVector Index);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Setting")
	int GeneratedNode = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	FVector GridSize = {20,20,20};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	FIntVector SpacingBetweenNode = {5,3,2};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting | Debug")
	bool DrawNodesSphere = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting | Debug")
	float DrawUpdate = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAStarNode> NodeClass;

	//UPROPERTY(BlueprintReadOnly)
	//TArray<TObjectPtr<UAStarNode>> NodeList;



protected:
	USceneComponent* SceneRoot;
private:	
	void ContructionIInitializeGrid();
	void OnConstruction(const FTransform& Transform) override;
	void BeginPlay() override;


#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//Draw and clear debug visual
	void ReDrawEditorDebugNodeSphere();
	void ClearEditorDebugNodeSphere();
#endif

	void GenerateGrid();
	void SpawnNode(const FIntVector& Point);
	void ConnectNeighbour();
	bool CheckNodeCollision(const FVector& WorldLocation);

	void UpdateNodeValidPath();

	/*Convert node world location to point*/
	FIntVector ConvertLocationToPoint(FVector Location) const;
	/*Convert Point To Relative Location*/
	FVector ConvertPointToLocation(FVector Point) const;

private:
	bool m_IsGeneratingGrid = false;
	
	//TArray<FNodeRealData> NodeDataList;

	TArray<TArray<TArray<FNodeRealData>>> NodeDataGrid;


};

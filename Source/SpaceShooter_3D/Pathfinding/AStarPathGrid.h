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
USTRUCT()
struct FAStarNodeData {
		GENERATED_BODY()
	
	public:
		UAStarNode* Node;
		UAStarNode* CameFrom;

		float gCost;
		float hCost;
		

		FAStarNodeData() : gCost(FLT_MAX), hCost(FLT_MAX), Node(nullptr), CameFrom(nullptr){
		}

		FAStarNodeData(UAStarNode* n) : gCost(FLT_MAX), hCost(FLT_MAX), Node(n) {
		}

		bool operator==(const FAStarNodeData& Other) const
		{
			return this->Node == Other.Node;
		}

		bool operator<(const FAStarNodeData& Other) const {
			return GetFCost() < Other.GetFCost();
		}

		const bool IsValidNode() const {
			return Node ? true : false;
		}

		const FVector GetLocation() const {
			if (Node) {
				return Node->GetComponentLocation();
			}
			return FVector::Zero();
		}

		const TArray<UAStarNode*> GetNeighbour() const {
			if (Node) {
				return Node->Neightbours;
			}
			return TArray<UAStarNode*>();
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
	UAStarNode* GetClosestNode(FVector Position) const;
	//TArray<FVector> FindPath(AAStarNode* StartPoint, AAStarNode* EndNode);


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	FVector GridSize = {20,20,20};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	FIntVector SpacingBetweenNode = {5,3,2};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	float TraceRadius = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting | Debug")
	bool DrawNodesSphere = false;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAStarNode> NodeClass;

//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	//TMap<FVector, FAStarNodeData> Grid;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UAStarNode>> NodeList;

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
	void SpawnNode(const FVector& Point);
	void ConnectNeighbour();
	bool CheckNodeCollision(const FVector& WorldLocation);

	void UpdateNodeValidPath();

private:
	bool IsGeneratingGrid = false;

	


};

// Fill out your copyright notice in the Description page of Project Settings.


#include "AStarPathGrid.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/LineBatchComponent.h"

// Sets default values
AAStarPathGrid::AAStarPathGrid()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void AAStarPathGrid::RefreshPathFinding()
{
	GenerateGrid();
}

UAStarNode* AAStarPathGrid::GetClosestNode(FVector Position) const
{
	float ShortestDistance = FLT_MAX;
	UAStarNode* OutNode = nullptr;
	for (UAStarNode* Node : NodeList) {
		if (Node && !Node->InvalidPath) {
			float CurrentDist = FVector::Dist(Position, Node->GetComponentLocation());
			if (CurrentDist < ShortestDistance) {
				OutNode = Node;
				ShortestDistance = CurrentDist;
			}

		}

	}

	return OutNode;
}

void AAStarPathGrid::OnConstruction(const FTransform& Transform)
{
	ContructionIInitializeGrid();
}

void AAStarPathGrid::BeginPlay()
{
	Super::BeginPlay();
	if (DrawNodesSphere)
		ReDrawEditorDebugNodeSphere();
}


void AAStarPathGrid::ContructionIInitializeGrid()
{
	UpdateNodeValidPath();
}

void AAStarPathGrid::SpawnNode(const FVector& Point)
{
	//Grid.Add(Point, FAStarNodeStruct(Point));
	UWorld* World = GetWorld();
	UAStarNode* NewNode = NewObject<UAStarNode>(this, UAStarNode::StaticClass());
	if (NewNode) {

		FVector NodeLocation = -GridSize + (Point * (GridSize * 2 / (FVector(SpacingBetweenNode) - FVector(1.0f))));

		NewNode->Point = FIntVector(Point);
		NewNode->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);
		NewNode->SetRelativeLocation(NodeLocation);
		NewNode->InvalidPath = CheckNodeCollision(NewNode->GetComponentLocation());

		NodeList.AddUnique(NewNode);

	}

}

void AAStarPathGrid::ConnectNeighbour()
{
	for (UAStarNode* CurrentNode : NodeList) {

		CurrentNode->Neightbours.Empty();

		const FVector ValuePlus = FVector(
			UKismetMathLibrary::Max(0, (SpacingBetweenNode.X - 1)),
			UKismetMathLibrary::Max(0, (SpacingBetweenNode.Y - 1)),
			UKismetMathLibrary::Max(0, (SpacingBetweenNode.Z - 1)));

		const FVector SpacingNeighbourVector = FVector((GridSize.X * 2 / ValuePlus.X), (GridSize.Y * 2 / ValuePlus.Y), (GridSize.Z * 2 / ValuePlus.Z));

		const TArray<FIntVector> NeighborOffsets = {
			FIntVector(1, 0, 0), FIntVector(-1, 0, 0),
			FIntVector(0, 1, 0), FIntVector(0, -1, 0),
			FIntVector(0, 0, 1), FIntVector(0, 0, -1),
			FIntVector(1, 0, 1), FIntVector(-1, 0, 1),
			FIntVector(1, 0, -1), FIntVector(-1, 0, -1),
			FIntVector(0, 1, 1), FIntVector(0, -1, 1),
			FIntVector(0, 1, -1), FIntVector(0, -1, -1),
			FIntVector(1, 1, 0), FIntVector(-1, 1, 0),
			FIntVector(1, -1, 0), FIntVector(-1, -1, 0),
			FIntVector(1, 1, 1), FIntVector(-1, 1, 1),
			FIntVector(1, -1, 1), FIntVector(-1, -1, 1),
			FIntVector(1, 1, -1), FIntVector(-1, 1, -1),
			FIntVector(1, -1, -1), FIntVector(-1, -1, -1)
			};

		//const TArray<FIntVector> DiagonalNeighborOffsets = {
		//	FIntVector(1, 1, 1), FIntVector(1, 1, -1), FIntVector(1, -1, 1), FIntVector(1, -1, -1),
		//	FIntVector(-1, 1, 1), FIntVector(-1, 1, -1), FIntVector(-1, -1, 1), FIntVector(-1, -1, -1)
		//};

		// Iterate through neighbor offsets
		for (const FIntVector& Offset : NeighborOffsets)
		{
			const FIntVector PointToFind = CurrentNode->Point + Offset;

			if (PointToFind.X == -1 || PointToFind.Y == -1 || PointToFind.Z == -1)
				continue;

			auto NeighbourNodeRef = NodeList.FindByPredicate([&PointToFind](const TObjectPtr<UAStarNode>& Node) -> bool {
				return Node && Node->Point == PointToFind;
				});

			if (NeighbourNodeRef) {
				if (UAStarNode* NeighborNode = NeighbourNodeRef->Get())
				{
					if (CurrentNode != NeighborNode) {
						CurrentNode->Neightbours.AddUnique(NeighborNode);
					}
				}
			}
		}
	}
}

bool AAStarPathGrid::CheckNodeCollision(const FVector& WorldLocation)
{
	FCollisionQueryParams CollisionParams;

	bool bBlockingHit = GetWorld()->SweepTestByChannel(
		WorldLocation,
		WorldLocation,
		FQuat::Identity,
		ECollisionChannel::ECC_Visibility,
		FCollisionShape::MakeSphere(TraceRadius),
		CollisionParams
	);

	return bBlockingHit;
}

void AAStarPathGrid::UpdateNodeValidPath()
{
	for (auto Node : NodeList) {
		if (Node)
			Node->InvalidPath = CheckNodeCollision(Node->GetComponentLocation());
	}
	ReDrawEditorDebugNodeSphere();

}

void AAStarPathGrid::GenerateGrid()
{
	if (!IsGeneratingGrid) {

		IsGeneratingGrid = true;
		//int XValuePlus = UKismetMathLibrary::Max(0, (SpacingBetweenNode.X - 1));
		//int YValuePlus = UKismetMathLibrary::Max(0, (SpacingBetweenNode.Y - 1));
		//int ZValuePlus = UKismetMathLibrary::Max(0, (SpacingBetweenNode.Z - 1));

		for (auto Node : NodeList) {
			if (Node)
				Node->DestroyComponent();
		}

		NodeList.Empty();

		for (int i = 0; i < SpacingBetweenNode.X; i++) {
			for (int j = 0; j < SpacingBetweenNode.Y; j++) {
				for (int k = 0; k < SpacingBetweenNode.Z; k++) {
					SpawnNode(FVector(i, j, k));
				}
			}

		}


		ConnectNeighbour();

		ReDrawEditorDebugNodeSphere();

		IsGeneratingGrid = false;

	}

}


#if WITH_EDITOR
void AAStarPathGrid::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!PropertyChangedEvent.Property)
		return;

	// Get the property that was modified
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	// Get the full path of the property that was edited
	const FString FullPath = PropertyChangedEvent.Property->GetFullName();


	if (FullPath.Contains(TEXT("/Script/CoreUObject.Vector")) || FullPath.Contains(TEXT("/Script/CoreUObject.IntVector")))
	{
		RefreshPathFinding();
	}

}

void AAStarPathGrid::ReDrawEditorDebugNodeSphere()
{
	//if (GEngine && GEngine->IsEditor())
	//{
	UWorld* World = GetWorld();


	if (World)
	{
		ClearEditorDebugNodeSphere();

		if (DrawNodesSphere) {
			for (const UAStarNode* N : NodeList) {
				if (N) {
					if (N->InvalidPath)
						DrawDebugSphere(World, N->GetComponentLocation(), TraceRadius, 2.0f, FColor::Red, true, -1, 0, 2);
					else
						DrawDebugSphere(World, N->GetComponentLocation(), TraceRadius, 2.0f, FColor::Green, true, -1, 0, 2);
				}
			}
		}
	}
	//}
}

void AAStarPathGrid::ClearEditorDebugNodeSphere()
{
	UWorld* World = GetWorld();

	ULineBatchComponent* LineBatcher = World->PersistentLineBatcher;
	if (LineBatcher)
	{
		LineBatcher->Flush(); // Clear all previous debug shapes
	}
}
#endif



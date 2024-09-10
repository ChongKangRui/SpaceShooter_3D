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

void AAStarPathGrid::OnConstruction(const FTransform& Transform)
{
	ContructionIInitializeGrid();
}

void AAStarPathGrid::BeginPlay()
{
	Super::BeginPlay();

	if (DrawUpdate > 0.0f) {
		FTimerDelegate MoveDelegate;
		FTimerHandle Handle;
		MoveDelegate.BindWeakLambda(this, [&]()
			{
				ReDrawEditorDebugNodeSphere();
			});

		GetWorld()->GetTimerManager().SetTimer(Handle, MoveDelegate, DrawUpdate, true);
	}
	RefreshPathFinding();
	UE_LOG(LogTemp, Error, TEXT("BeginPlay IsEmpty? %s"), NodeDataGrid.IsEmpty() ? TEXT("True") : TEXT("False"));
}


void AAStarPathGrid::ContructionIInitializeGrid()
{
	UpdateNodeValidPath();
}

void AAStarPathGrid::RefreshPathFinding()
{
	GenerateGrid();
}

FNodeRealData& AAStarPathGrid::GetClosestNode(FVector Position)
{
	FIntVector Point = ConvertLocationToPoint(Position);

	FNodeRealData& OutNode = GetNode(Point);
	FNodeRealData FoundNode = OutNode;

	float shortestDistance = FVector::Dist(Position, OutNode.Location);

	for (FIntVector Neighbour : FoundNode.Neightbours) {
		FNodeRealData& CurrentNode = GetNode(Neighbour);
		float newDistance = FVector::Dist(Position, CurrentNode.Location);
		if (newDistance < shortestDistance) {
			UE_LOG(LogTemp, Error, TEXT("Meet condition for closest Node"));
			shortestDistance = newDistance;
			OutNode = CurrentNode;
		}
	}

	return OutNode;
}

FNodeRealData& AAStarPathGrid::GetNode(FIntVector Index) {
	static FNodeRealData EmptyNode;

	if (NodeDataGrid.IsValidIndex(Index.X) && NodeDataGrid[Index.X].IsValidIndex(Index.Y) && NodeDataGrid[Index.X][Index.Y].IsValidIndex(Index.Z)) {

		return NodeDataGrid[Index.X][Index.Y][Index.Z];
	}

	return EmptyNode;
}

FVector AAStarPathGrid::GetRandomLocationWithinRange(const FVector Center, const float MinimumRange, const float MaximumRange)
{
	FVector RandomLocation = FMath::VRand() * FMath::FRandRange(MinimumRange, MaximumRange);
	
	return Center + RandomLocation;
}

FIntVector AAStarPathGrid::ConvertLocationToPoint(FVector Location) const
{
	FVector Divisor = GridSize * 2 / (FVector(SpacingBetweenNode) - FVector(1.0f));
	FVector Point = ((Location - GetActorLocation()) + GridSize) / Divisor;

	return FIntVector(Point);
}


FVector AAStarPathGrid::ConvertPointToLocation(FVector Point) const
{
	return -GridSize + (Point * (GridSize * 2 / (FVector(SpacingBetweenNode) - FVector(1.0f))));
}

void AAStarPathGrid::SpawnNode(const FIntVector& Point)
{

	FVector NodeLocation = ConvertPointToLocation(FVector(Point));
	FNodeRealData data(GetActorLocation() + NodeLocation);

	if (CheckNodeCollision(data.Location)) {
		data.Status = ENodeStatus::InvalidPath;
	}
	else
		data.Status = ENodeStatus::AllowToPass;

	for (int x = -1; x < 1; x++) {
		for (int y = -1; y < 1; y++) {
			for (int z = -1; z < 1; z++) {
				FIntVector GridIndex = Point + FIntVector(x, y, z);

				if ((GridIndex.X == -1 || GridIndex.Y == -1 || GridIndex.Z == -1)
					|| (GridIndex.X > SpacingBetweenNode.X || GridIndex.Y > SpacingBetweenNode.Y || GridIndex.Z > SpacingBetweenNode.Z))
					continue;

				if (GridIndex != Point) {
					NodeDataGrid[GridIndex.X][GridIndex.Y][GridIndex.Z].Neightbours.AddUnique(Point);
					data.Neightbours.AddUnique(GridIndex);
				}

			}
		}
	}

	NodeDataGrid[Point.X][Point.Y][Point.Z] = data;
	GeneratedNode++;

}

void AAStarPathGrid::ConnectNeighbour()
{
	//for (UAStarNode* CurrentNode : NodeList) {
	//for (FNodeRealData& node : NodeDataList) {
	//
	//	if (node.CheckIsNodeInvalid())
	//		continue;
	//
	//	node.Neightbours.Empty();
	//
	//	const FVector ValuePlus = FVector(
	//		UKismetMathLibrary::Max(0, (SpacingBetweenNode.X - 1)),
	//		UKismetMathLibrary::Max(0, (SpacingBetweenNode.Y - 1)),
	//		UKismetMathLibrary::Max(0, (SpacingBetweenNode.Z - 1)));
	//
	//	const FVector SpacingNeighbourVector = FVector((GridSize.X * 2 / ValuePlus.X), (GridSize.Y * 2 / ValuePlus.Y), (GridSize.Z * 2 / ValuePlus.Z));
	//
	//	const TArray<FIntVector> NeighborOffsets = {
	//		FIntVector(1, 0, 0), FIntVector(-1, 0, 0),
	//		FIntVector(0, 1, 0), FIntVector(0, -1, 0),
	//		FIntVector(0, 0, 1), FIntVector(0, 0, -1),
	//		FIntVector(1, 0, 1), FIntVector(-1, 0, 1),
	//		FIntVector(1, 0, -1), FIntVector(-1, 0, -1),
	//		FIntVector(0, 1, 1), FIntVector(0, -1, 1),
	//		FIntVector(0, 1, -1), FIntVector(0, -1, -1),
	//		FIntVector(1, 1, 0), FIntVector(-1, 1, 0),
	//		FIntVector(1, -1, 0), FIntVector(-1, -1, 0),
	//		FIntVector(1, 1, 1), FIntVector(-1, 1, 1),
	//		FIntVector(1, -1, 1), FIntVector(-1, -1, 1),
	//		FIntVector(1, 1, -1), FIntVector(-1, 1, -1),
	//		FIntVector(1, -1, -1), FIntVector(-1, -1, -1)
	//	};
	//
	//
	//	// Iterate through neighbor offsets
	//	for (const FIntVector& Offset : NeighborOffsets)
	//	{
	//		const FIntVector PointToFind = ConvertLocationToPoint(node.Location) + Offset;
	//		UE_LOG(LogTemp, Error, TEXT("%s"), *PointToFind.ToString());
	//		
	//		//if (PointToFind.X == -1 || PointToFind.Y == -1 || PointToFind.Z == -1)
	//		//	continue;
	//		
	//		//auto NeighbourNodeRef = NodeDataList.FindByPredicate([this, &PointToFind](const FNodeRealData& Node) -> bool {
	//		//	return ConvertLocationToPoint(Node.Location) == PointToFind;
	//		//	});
	//		//
	//		//if (NeighbourNodeRef) {
	//		//
	//		//	if (node != *NeighbourNodeRef) {
	//		//		node.Neightbours.AddUnique(*NeighbourNodeRef);
	//		//	}
	//		//
	//		//}
	//	}
	//}	
}

bool AAStarPathGrid::CheckNodeCollision(const FVector& WorldLocation)
{
	FCollisionQueryParams CollisionParams;

	bool bBlockingHit = GetWorld()->SweepTestByChannel(
		WorldLocation,
		WorldLocation,
		FQuat::Identity,
		ECollisionChannel::ECC_Visibility,
		FCollisionShape::MakeBox(FVector3f(GridSize.X / SpacingBetweenNode.X, GridSize.Y / SpacingBetweenNode.Y, GridSize.Z / SpacingBetweenNode.Z)),
		CollisionParams
	);

	return bBlockingHit;
}

void AAStarPathGrid::UpdateNodeValidPath()
{
	for (int i = 0; i < SpacingBetweenNode.X; i++) {

		for (int j = 0; j < SpacingBetweenNode.Y; j++) {

			for (int k = 0; k < SpacingBetweenNode.Z; k++) {

				if (NodeDataGrid.IsValidIndex(i) && NodeDataGrid[i].IsValidIndex(j) && NodeDataGrid[i][j].IsValidIndex(k)) {
					FNodeRealData& Node = NodeDataGrid[i][j][k];
					if (CheckNodeCollision(Node.Location)) {
						Node.Status = ENodeStatus::InvalidPath;
					}
					else {
						Node.Status = ENodeStatus::AllowToPass;
					}
				}
			}
		}

	}

	ReDrawEditorDebugNodeSphere();

}


void AAStarPathGrid::GenerateGrid()
{
	if (!m_IsGeneratingGrid) {

		m_IsGeneratingGrid = true;

		NodeDataGrid.Empty();

		GeneratedNode = 0;

		NodeDataGrid.SetNum(SpacingBetweenNode.X);
		for (int i = 0; i < SpacingBetweenNode.X; i++) {
			NodeDataGrid[i].SetNum(SpacingBetweenNode.Y);
			for (int j = 0; j < SpacingBetweenNode.Y; j++) {
				NodeDataGrid[i][j].SetNum(SpacingBetweenNode.Z);
				for (int k = 0; k < SpacingBetweenNode.Z; k++) {
					SpawnNode(FIntVector(i, j, k));
				}
			}

		}

		//ConnectNeighbour();
		ReDrawEditorDebugNodeSphere();

		UE_LOG(LogTemp, Error, TEXT("%s"), NodeDataGrid.IsEmpty() ? TEXT("True") : TEXT("False"));

		m_IsGeneratingGrid = false;
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

	UWorld* World = GetWorld();

	if (World)
	{
		ClearEditorDebugNodeSphere();

		if (DrawNodesSphere) {
			for (int i = 0; i < SpacingBetweenNode.X; i++) {

				for (int j = 0; j < SpacingBetweenNode.Y; j++) {

					for (int k = 0; k < SpacingBetweenNode.Z; k++) {

						const FNodeRealData& Node = NodeDataGrid[i][j][k];

						if (Node.CheckIsNodeInvalid())
							DrawDebugBox(World, Node.Location, FVector(GridSize.X / SpacingBetweenNode.X, GridSize.Y / SpacingBetweenNode.Y, GridSize.Z / SpacingBetweenNode.Z), FColor::Red, true, -1, 0, 2);
						else if (Node.CheckIsNodeOccupied()) {
							DrawDebugBox(World, Node.Location, FVector(GridSize.X / SpacingBetweenNode.X, GridSize.Y / SpacingBetweenNode.Y, GridSize.Z / SpacingBetweenNode.Z), FColor::Blue, true, -1, 0, 2);
						}
						else
							DrawDebugBox(World, Node.Location, FVector(GridSize.X / SpacingBetweenNode.X, GridSize.Y / SpacingBetweenNode.Y, GridSize.Z / SpacingBetweenNode.Z), FColor::Green, true, -1, 0, 2);

					}
				}

			}

		}
	}

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



// Fill out your copyright notice in the Description page of Project Settings.


#include "AStarPathGrid.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/LineBatchComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
AAStarPathGrid::AAStarPathGrid()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	BoxComponent->SetupAttachment(SceneRoot);
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

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AAStarPathGrid::OnComponentBeginOverlap);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &AAStarPathGrid::OnComponentEndOverlap);

	RefreshPathFinding();
	
}

uint32 GetTypeHash(const FAStarNodeData& AStarNode)
{
	if (AStarNode.Node)
		return GetTypeHash(*AStarNode.Node);

	return 0;
}

uint32 GetTypeHash(const FNodeRealData& AStarNode) {
	
	return GetTypeHash(AStarNode.Location);
}


void AAStarPathGrid::ContructionIInitializeGrid()
{
	UpdateNodeValidPath();
}

void AAStarPathGrid::RefreshPathFinding()
{
	GenerateGrid();
}

FNodeRealData* AAStarPathGrid::GetClosestNode(FVector Position)
{
	if (m_IsGeneratingGrid)
		return nullptr;


	FIntVector Point = ConvertLocationToPoint(Position);

	Point = FIntVector(FMath::Clamp(Point.X,0, SpacingBetweenNode.X - 1), FMath::Clamp(Point.Y, 0, SpacingBetweenNode.Y - 1), FMath::Clamp(Point.Z, 0, SpacingBetweenNode.Z - 1));

	UE_LOG(LogTemp, Error, TEXT("Point to find %s"), *Point.ToString());

	FNodeRealData* OutNode = GetNode(Point);
	FNodeRealData* FoundNode = OutNode;

	if (OutNode) {
		float shortestDistance = FVector::Dist(Position, OutNode->Location);

		for (FIntVector Neighbour : OutNode->Neightbours) {
			FNodeRealData* CurrentNode = GetNode(Neighbour);

			if (!CurrentNode)
				continue;

			float newDistance = FVector::Dist(Position, CurrentNode->Location);
			if (newDistance < shortestDistance) {
				shortestDistance = newDistance;
				OutNode = CurrentNode;
			}
		}
	}

	return OutNode;
}

FNodeRealData* AAStarPathGrid::GetNode(FIntVector Index) {
	
	if (m_IsGeneratingGrid)
		return nullptr;


	if (NodeDataGrid.IsValidIndex(Index.X) && NodeDataGrid[Index.X].IsValidIndex(Index.Y) && NodeDataGrid[Index.X][Index.Y].IsValidIndex(Index.Z)) {

		return NodeDataGrid[Index.X][Index.Y][Index.Z];
	}
	UE_LOG(LogTemp, Error, TEXT("Invalid Node"));
	return nullptr;
}

FVector AAStarPathGrid::GetRandomLocationWithinRange(const FVector Center, const float MinimumRange, const float MaximumRange)
{
	FVector RandomLocation = FMath::VRand() * FMath::FRandRange(MinimumRange, MaximumRange);

	DrawDebugSphere(GetWorld(), Center + RandomLocation, 300.0f, 4.0f, FColor::Yellow, false, 10.0f, 0, 2);
	
	return Center + RandomLocation;
}

void AAStarPathGrid::SetNodeStatus(FIntVector Index, ENodeStatus occupiedStatus)
{
	if (NodeDataGrid.IsValidIndex(Index.X) && NodeDataGrid[Index.X].IsValidIndex(Index.Y) && NodeDataGrid[Index.X][Index.Y].IsValidIndex(Index.Z)) {

		NodeDataGrid[Index.X][Index.Y][Index.Z]->Status = occupiedStatus;
	}
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
	FNodeRealData* data = new FNodeRealData(GetActorLocation() + NodeLocation, Point);


	if (CheckNodeCollision(data->Location)) {
		data->Status = ENodeStatus::InvalidPath;
	}
	else
		data->Status = ENodeStatus::AllowToPass;

	for (int x = -1; x < 1; x++) {
		for (int y = -1; y < 1; y++) {
			for (int z = -1; z < 1; z++) {
				FIntVector GridIndex = Point + FIntVector(x, y, z);

				if ((GridIndex.X == -1 || GridIndex.Y == -1 || GridIndex.Z == -1)
					|| (GridIndex.X > SpacingBetweenNode.X || GridIndex.Y > SpacingBetweenNode.Y || GridIndex.Z > SpacingBetweenNode.Z))
					continue;

				if (GridIndex != Point) {
					NodeDataGrid[GridIndex.X][GridIndex.Y][GridIndex.Z]->Neightbours.AddUnique(Point);
					data->Neightbours.AddUnique(GridIndex);
				}

			}
		}
	}

	NodeDataGrid[Point.X][Point.Y][Point.Z] = data;
	GeneratedNode++;

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
					FNodeRealData* Node = NodeDataGrid[i][j][k];

					if (!Node)
						continue;

					if (CheckNodeCollision(Node->Location)) {
						Node->Status = ENodeStatus::InvalidPath;
					}
					else {
						Node->Status = ENodeStatus::AllowToPass;
					}
				}
			}
		}

	}

	ReDrawEditorDebugNodeSphere();

}

void AAStarPathGrid::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor) {
		if (OtherActor->ActorHasTag("Player")) {
			UE_LOG(LogTemp, Error, TEXT("Dynamic Player Location"));
			SetActorLocation(OtherActor->GetActorLocation());
			RefreshPathFinding();
		}
	}
}

void AAStarPathGrid::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
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

		UE_LOG(LogTemp, Error, TEXT("Generate node %s"), NodeDataGrid.IsEmpty() ? TEXT("Failed") : TEXT("Success"));

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

						const FNodeRealData* Node = NodeDataGrid[i][j][k];
						if (!Node)
							continue;

						if (Node->CheckIsNodeInvalid())
							DrawDebugBox(World, Node->Location, FVector(GridSize.X / SpacingBetweenNode.X, GridSize.Y / SpacingBetweenNode.Y, GridSize.Z / SpacingBetweenNode.Z), FColor::Red, true, -1, 0, 2);
						else if (Node->CheckIsNodeOccupied()) {
							DrawDebugBox(World, Node->Location, FVector(GridSize.X / SpacingBetweenNode.X, GridSize.Y / SpacingBetweenNode.Y, GridSize.Z / SpacingBetweenNode.Z), FColor::Blue, true, -1, 0, 2);
						}
						else
							DrawDebugBox(World, Node->Location, FVector(GridSize.X / SpacingBetweenNode.X, GridSize.Y / SpacingBetweenNode.Y, GridSize.Z / SpacingBetweenNode.Z), FColor::Green, true, -1, 0, 2);

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



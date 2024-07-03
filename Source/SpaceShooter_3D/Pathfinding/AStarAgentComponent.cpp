// Fill out your copyright notice in the Description page of Project Settings.


#include "AStarAgentComponent.h"
#include "AStarPathGrid.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// Sets default values for this component's properties
UAStarAgentComponent::UAStarAgentComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UAStarAgentComponent::BeginPlay()
{
	Super::BeginPlay();
	TArray<AActor*> PathGridActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAStarPathGrid::StaticClass(), PathGridActors);

	float Dist = FLT_MAX;
	AAStarPathGrid* BestPathGrid;

	for (AActor* A : PathGridActors) {
		if (AAStarPathGrid* PG = Cast<AAStarPathGrid>(A)) {

			float shortestDist = FVector::Dist(PG->GetActorLocation(), GetOwner()->GetActorLocation());
			if (shortestDist < Dist) {
				shortestDist = Dist;
				BestPathGrid = PG;
			}
		}

	}

	m_PathGrid = BestPathGrid;


}

TArray<FVector> UAStarAgentComponent::ReconstructPath(const UAStarNode* Goal, const TArray<FAStarNodeData> NodeList)
{
	TArray<FVector> Path;

	auto EndNodeData = NodeList.FindByPredicate([&](const FAStarNodeData& NodeData) {
		return NodeData.Node == Goal;
		});
	

	FAStarNodeData CurrentNodeData;

	if (EndNodeData)
		CurrentNodeData = *EndNodeData;
	else
		UE_LOG(LogTemp, Error, TEXT("Invalid goal end node"));

	//Path.Add(Goal.GetLocation());

	while (CurrentNodeData.CameFrom)
	{
		Path.Insert(CurrentNodeData.GetLocation(), 0); // Insert at the beginning
		auto NodeRef = NodeList.FindByPredicate([&](const FAStarNodeData& NodeData) {
			return NodeData.Node == CurrentNodeData.CameFrom;
			});

		if (!NodeRef) {
			break;
		}
		CurrentNodeData = *NodeRef;
	}

	for (int i = 1; i < Path.Num();i++) {
		DrawDebugLine(GetWorld(), Path[i - 1], Path[i], FColor::Red, false, 10.0f);
	}
	

	return Path;
}

void UAStarAgentComponent::AgentMove(TArray<FVector>& Path)
{
	if (Path.Num() == 0) return;
	

	FVector NextWaypoint = Path[0];
	Path.RemoveAt(0);

	//ToDo: make a better movement function like using FInterp
	//Right now should use debug line to show the path

	FTimerDelegate MoveDelegate;
	MoveDelegate.BindWeakLambda(this, [&Path]()
	{
			
	});
	

	if (Path.Num() > 0)
	{
	
	}
	else
	{
		m_AgentStatus = EPathfindingStatus::Success;
		UE_LOG(LogTemp, Error, TEXT("Path following complete"));
	}
}

void UAStarAgentComponent::MoveTo(FVector Goal)
{
	if (!m_PathGrid) {
		UE_LOG(LogTemp, Error, TEXT("Invalid Path Grid Ref"));
		return;
	}

	if (m_AgentStatus == EPathfindingStatus::InProgress)
		return;

	UAStarNode* StartNode = m_PathGrid->GetClosestNode(GetOwner()->GetActorLocation());
	UAStarNode* GoalNode = m_PathGrid->GetClosestNode(Goal);

	if (StartNode == GoalNode) {
		m_AgentStatus = EPathfindingStatus::Failed;
		UE_LOG(LogTemp, Error, TEXT("failed both same"));
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(m_PathFindingHandle);

	TArray<FAStarNodeData> OpenList;
	TArray<FAStarNodeData> ClosedList;

	OpenList.Add(FAStarNodeData(StartNode));

	FAStarNodeData CurrentNodeData = OpenList[0];

	UE_LOG(LogTemp, Error, TEXT("Hey start loop"));

	CurrentNodeData.gCost = 0;
	CurrentNodeData.hCost = FVector::Dist(StartNode->GetComponentLocation(), GoalNode->GetComponentLocation());

	while (OpenList.Num() > 0) {

		FAStarNodeData Current;
		OpenList.HeapPop(Current, true);

		ClosedList.Add(Current);

		if (Current.IsValidNode()) {

			for (UAStarNode* Neighbor : Current.GetNeighbour())
			{
				if (!Neighbor)
					continue;

				if (ClosedList.Contains(Neighbor) || Neighbor->InvalidPath)
				{
					continue;
				}

				FAStarNodeData NeighbourData(Neighbor);

				float NewGCost = Current.gCost + FVector::Dist(Current.GetLocation(), NeighbourData.GetLocation()) + Neighbor->cost;

				if (NewGCost < NeighbourData.gCost || !OpenList.Contains(Neighbor))
				{

					NeighbourData.gCost = NewGCost;
					NeighbourData.hCost = FVector::Dist(NeighbourData.GetLocation(), GoalNode->GetComponentLocation());
					NeighbourData.CameFrom = Current.Node;

					if (!OpenList.Contains(NeighbourData))
					{
						OpenList.HeapPush(NeighbourData, [](const FAStarNodeData& A, const FAStarNodeData& B) { return A < B; });

					}
				}
			}
		}
	}
	UE_LOG(LogTemp, Error, TEXT("Success Path Finding end %i"), ClosedList.Num());
	

	ReconstructPath(GoalNode,ClosedList);
}

EPathfindingStatus UAStarAgentComponent::GetAgentStatus() const
{
	return m_AgentStatus;
}


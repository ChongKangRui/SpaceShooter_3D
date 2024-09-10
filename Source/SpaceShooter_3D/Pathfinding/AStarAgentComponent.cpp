// Fill out your copyright notice in the Description page of Project Settings.


#include "AStarAgentComponent.h"
#include "AStarPathGrid.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Character/SpaceShooter_3DCharacter.h"


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
	m_Agent = Cast<ASpaceShooter_3DCharacter>(GetOwner());

}

TArray<FAStarNodeData> UAStarAgentComponent::ReconstructPath(const FNodeRealData& Goal, const TArray<FAStarNodeData> NodeList)
{
	TArray<FAStarNodeData> Path;

	auto EndNodeData = NodeList.FindByPredicate([&](const FAStarNodeData& NodeData) {
		return *NodeData.Node == Goal;
		});
	

	FAStarNodeData CurrentNodeData;

	if (EndNodeData)
		CurrentNodeData = *EndNodeData;
	else
		UE_LOG(LogTemp, Error, TEXT("Invalid goal end node"));

	while (CurrentNodeData.CameFrom && !CurrentNodeData.CameFrom->CheckIsNodeInvalid())
	{
		Path.Insert(CurrentNodeData, 0); // Insert at the beginning
		auto NodeRef = NodeList.FindByPredicate([&](const FAStarNodeData& NodeData) {
			return NodeData.Node == CurrentNodeData.CameFrom;
			});

		if (!NodeRef) {
			break;
		}
		CurrentNodeData = *NodeRef;
	}

	for (int i = 1; i < Path.Num();i++) {
		DrawDebugLine(GetWorld(), Path[i - 1].GetLocation(), Path[i].GetLocation(), FColor::Red, false, 10.0f);
	}
	

	return Path;
}

void UAStarAgentComponent::AgentMove()
{
	if (m_Path.Num() == 0) { 
		m_AgentStatus = EPathfindingStatus::Success;
		return; 
	}

	if (!m_Agent->GetController()) {
		UE_LOG(LogTemp, Error, TEXT("Invalid Agent Controller"));
		return;
	}
	
	m_AgentStatus = EPathfindingStatus::InProgress;

	//ToDo: make a better movement function like using FInterp

	FVector AgentLocation = m_Agent->GetActorLocation();

	FVector NextWaypoint = m_Path[0].GetLocation();
	FVector AgentNewPosition = AgentLocation + (m_Agent->GetActorForwardVector() * FlyingSpeed * GetWorld()->GetDeltaSeconds());

	FVector DirectionToTarget = NextWaypoint - AgentLocation;
	FVector DirectionToTargetNormalize = DirectionToTarget.GetSafeNormal();

	float YawRadian = FMath::Atan2(DirectionToTargetNormalize.Y, DirectionToTargetNormalize.X);
	float YawAngle = FMath::RadiansToDegrees(YawRadian);

	float PitchRadian = FMath::Atan2(DirectionToTargetNormalize.Z, FVector(DirectionToTargetNormalize.X, DirectionToTargetNormalize.Y, 0).Size());
	float PitchAngle = FMath::RadiansToDegrees(PitchRadian);

	FRotator TargetRotation = FRotator(PitchAngle, YawAngle, 0);
	FRotator AgentNewRotation = FMath::RInterpTo(m_Agent->GetActorRotation(),
		TargetRotation, GetWorld()->GetDeltaSeconds(), RotationSpeed);

	m_Agent->SetActorLocationAndRotation(AgentNewPosition, AgentNewRotation);
	
	if (FVector::Dist(AgentLocation, NextWaypoint) <= 50.0f) {

		//m_Path[0].Node->Status = ENodeStatus::AllowToPass;
		m_Path.RemoveAt(0);

		if (m_Path.Num() != 0) {
			/*If path had occupied, recalculate it, otherwise, move to and set the node to be occupied*/
			if (m_Path[0].Node) {
				if (m_Path[0].Node->CheckIsNodeOccupied()) {
					/*If this is last node and being occupied, we should stop then*/
					if(m_Path.Num() > 1)
						MoveTo(m_Path[m_Path.Num() - 1].GetLocation());
				}
				else {
					m_Path[0].Node->Status = ENodeStatus::ShipOccupied;
				}
			}
		}
	}
	
	FTimerDelegate MoveDelegate;
	MoveDelegate.BindWeakLambda(this, [&]()
	{
		AgentMove();
	});

	GetWorld()->GetTimerManager().SetTimer(m_PathFindingHandle, MoveDelegate, GetWorld()->GetDeltaSeconds(), false);
}

void UAStarAgentComponent::MoveTo(FVector Goal)
{
	if (!m_Agent) {
		UE_LOG(LogTemp, Error, TEXT("Invalid Agent Reference"));
		return;
	}

	if (!m_PathGrid) {
		UE_LOG(LogTemp, Error, TEXT("Invalid Path Grid Ref"));
		return;
	}

	FNodeRealData& StartNode = m_PathGrid->GetClosestNode(GetOwner()->GetActorLocation());
	FNodeRealData& GoalNode = m_PathGrid->GetClosestNode(Goal);

	UE_LOG(LogTemp, Error, TEXT("Goal Location %s and Goal Node %s"), *GoalNode.Location.ToString(), *Goal.ToString());

	if (m_AgentStatus == EPathfindingStatus::InProgress){
		if (GoalNode.Location == m_Path[m_Path.Num() - 1].GetLocation()) {
			return;
		}
	}

	m_Path.Empty();

	if (StartNode == GoalNode) {
		m_AgentStatus = EPathfindingStatus::Failed;
		UE_LOG(LogTemp, Error, TEXT("failed to move, start node == goal node or Both node invalid"));
		return;
	}

	DrawDebugSphere(GetWorld(), StartNode.Location, 300.0f, 4.0f, FColor::Yellow, true, 10.0f, 0, 2);
	DrawDebugSphere(GetWorld(), GoalNode.Location, 300.0f, 4.0f, FColor::Green, true, 10.0f, 0, 2);

	GetWorld()->GetTimerManager().ClearTimer(m_PathFindingHandle);

	TArray<FAStarNodeData> OpenList;
	TArray<FAStarNodeData> ClosedList;

	FAStarNodeData CurrentNodeData(StartNode);

	CurrentNodeData.gCost = 0;
	CurrentNodeData.hCost = FVector::Dist(StartNode.Location, GoalNode.Location);

	OpenList.Add(CurrentNodeData);

	while (OpenList.Num() > 0) {
	
		FAStarNodeData Current;
		OpenList.HeapPop(Current, true);
	
		ClosedList.Add(Current);
	
		if (Current.Node == &GoalNode)
		{
			m_Path = ReconstructPath(GoalNode, ClosedList);
			AgentMove();
			//m_AgentStatus = EPathfindingStatus::Success;
			return;
		}
	
		if (Current.IsValidNode()) {
	
			//UE_LOG(LogTemp, Error, TEXT("Neighbour Number %i"), Current.GetNeighbour().Num());
			for (FIntVector& NeighborIndex : Current.GetNeighbour())
			{
				FNodeRealData& Neighbor = m_PathGrid->GetNode(NeighborIndex);
				if (Neighbor.CheckIsNodeInvalid())
					continue;
	
				if (ClosedList.Contains(Neighbor) || Neighbor.CheckIsNodeInvalid() || Neighbor.CheckIsNodeOccupied())
				{
					continue;
				}
	
				FAStarNodeData NeighbourData(Neighbor);
	
				/*The one is neighbour cost, for now no way to modify it at the moment*/
				float NewGCost = Current.gCost + FVector::Dist(Current.GetLocation(), NeighbourData.GetLocation()) + 1;
	
				if (NewGCost < NeighbourData.gCost)
				{
	
					NeighbourData.gCost = NewGCost;
					NeighbourData.hCost = FVector::Dist(NeighbourData.GetLocation(), GoalNode.Location);
					NeighbourData.CameFrom = Current.Node;
	
					if (!OpenList.Contains(NeighbourData))
					{
						OpenList.HeapPush(NeighbourData, [](const FAStarNodeData& A, const FAStarNodeData& B) { return A < B; });
	
					}
				}
			}
		}
	}
	//UE_LOG(LogTemp, Error, TEXT("Success Path Finding end %i"), ClosedList.Num());
	

	//m_Path = ReconstructPath(GoalNode,ClosedList);
	//AgentMove();
}

EPathfindingStatus UAStarAgentComponent::GetAgentStatus() const
{
	return m_AgentStatus;
}

AAStarPathGrid* UAStarAgentComponent::GetGridReference() const
{
	return m_PathGrid;
}


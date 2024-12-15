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
	SetComponentTickInterval(0.5f);
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

void UAStarAgentComponent::TickComponent(float Delta, ELevelTick type, FActorComponentTickFunction* func)
{
	Super::TickComponent(Delta, type, func);
	if (m_PathGrid && m_Agent) {
		//FIntVector curPoint = m_PathGrid->ConvertLocationToPoint(m_Agent->GetActorLocation());
		//if (m_PreviousLocation == curPoint) {
		//	return;
		//}
		//
		//m_PathGrid->SetNodeStatus(curPoint, ENodeStatus::ShipOccupied);
		//m_PathGrid->SetNodeStatus(m_PreviousLocation, ENodeStatus::AllowToPass);
		//m_PreviousLocation = curPoint;


	}
}

TArray<FVector> UAStarAgentComponent::ReconstructPath(const FNodeRealData& Goal, const TArray<TArray<TArray<FAStarNodeData>>> NodeList)
{
	TArray<FVector> Path;

	if (!m_PathGrid) {
		UE_LOG(LogTemp, Error, TEXT("Invalid path grid ref"));

		return Path;
	}

	FIntVector GoalsPoint = m_PathGrid->ConvertLocationToPoint(Goal.Location);

	FAStarNodeData CurrentNodeData = NodeList[GoalsPoint.X][GoalsPoint.Y][GoalsPoint.Z];

	while (CurrentNodeData.CameFrom.X != -1)
	{
		Path.Insert(CurrentNodeData.GetLocation(), 0); // Insert at the beginning

		CurrentNodeData.Node->AddOccupiingAgent(GetOwner(), CurrentNodeData.TimeToReach);

		UE_LOG(LogTemp, Error, TEXT("TimeToReach %f, CameFromPoint %s"), CurrentNodeData.TimeToReach, *CurrentNodeData.CameFrom.ToString());

		/*Problem hereerererer*/
		//auto NodeRef = NodeList.FindByPredicate([&](const FAStarNodeData& NodeData) {
		//	UE_LOG(LogTemp, Error, TEXT("Is Valid Node %s"), NodeData.Node ? TEXT("True") : TEXT("False"));
		//	return NodeData.Node == CurrentNodeData.CameFrom;
		//	});

		auto cameFrom = NodeList[CurrentNodeData.CameFrom.X][CurrentNodeData.CameFrom.Y][CurrentNodeData.CameFrom.Z];

		//if (!NodeRef) {
		//	break;
		//}
		CurrentNodeData = cameFrom;
	}



	return EnableBezierPath ? GenerateBezierPath(Path) : Path;
}

void UAStarAgentComponent::AgentMove()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AgentMove);
	if (m_Path.Num() == 0) {
		SetPathfindingState(EPathfindingStatus::Failed);
		//UE_LOG(LogTemp, Error, TEXT("Agent Finish Movement"));
		return;
	}

	if (!m_Agent->GetController()) {
		UE_LOG(LogTemp, Error, TEXT("Invalid Agent Controller"));
		return;
	}

	SetPathfindingState(EPathfindingStatus::InProgress);

	//ToDo: make a better movement function like using FInterp

	FVector AgentLocation = m_Agent->GetActorLocation();

	const FVector& NextWaypoint = m_Path[0];
	const FVector& direction = NextWaypoint - AgentLocation;

	const FVector& AgentNewPosition = AgentLocation + (direction.GetSafeNormal() * FlyingSpeed * GetWorld()->GetDeltaSeconds());

	const FVector& DirectionToTarget = NextWaypoint - AgentLocation;
	const FVector& DirectionToTargetNormalize = DirectionToTarget.GetSafeNormal();

	float YawRadian = FMath::Atan2(DirectionToTargetNormalize.Y, DirectionToTargetNormalize.X);
	float YawAngle = FMath::RadiansToDegrees(YawRadian);

	float PitchRadian = FMath::Atan2(DirectionToTargetNormalize.Z, FVector(DirectionToTargetNormalize.X, DirectionToTargetNormalize.Y, 0).Size());
	float PitchAngle = FMath::RadiansToDegrees(PitchRadian);

	//FRotator TargetRotation = FRotator(PitchAngle, YawAngle, 0);
	//FRotator AgentNewRotation = FMath::RInterpTo(m_Agent->GetActorRotation(),
	//	TargetRotation, GetWorld()->GetDeltaSeconds(), RotationSpeed);

	//m_Agent->SetActorLocationAndRotation(AgentNewPosition, AgentNewRotation);
	m_Agent->SetActorLocation(AgentNewPosition);
	OnAgentMoving.Broadcast(NextWaypoint);

	if (FVector::Dist(AgentLocation, NextWaypoint) <= AcceptableRange) {

		auto NextWayPointNode = m_PathGrid->GetNode(m_PathGrid->ConvertLocationToPoint(NextWaypoint));
		if (FVector::Dist(AgentLocation, NextWayPointNode->Location) <= AcceptableRange) {
			NextWayPointNode->RemoveOccupiingAgent(GetOwner());
		}

		if (m_Path.Num() == 0) {
			SetPathfindingState(EPathfindingStatus::Success);
			UE_LOG(LogTemp, Error, TEXT("Agent Finish Movement"));
			return;
		}

		m_Path.RemoveAt(0);

	}

	FTimerDelegate MoveDelegate;
	MoveDelegate.BindWeakLambda(this, [&]()
		{
			AgentMove();
		});

	GetWorld()->GetTimerManager().SetTimer(m_PathFindingHandle, MoveDelegate, GetWorld()->GetDeltaSeconds(), false);
}

void UAStarAgentComponent::DrawDebugPath(const TArray<FVector>& path)
{
	if (EnablePathFindingDebug) {
		for (int i = 1; i < path.Num(); i++) {
			DrawDebugLine(GetWorld(), path[i - 1], path[i], FColor::Red, false, 10.0f);
		}
	}
}

void UAStarAgentComponent::SetPathfindingState(const EPathfindingStatus& newStatus)
{
	m_AgentStatus = newStatus;
	OnPathfindingStateChanged.Broadcast(newStatus);
}

TArray<FVector> UAStarAgentComponent::GenerateBezierPath(TArray<FVector> path)
{

	TArray<FVector> BezierPath;

	// Ensure we have enough points to create at least one cubic Bezier curve
	if (path.Num() < 4)
	{
		return path; // Not enough points for a cubic Bezier curve
	}

	for (int32 i = 0; i < path.Num() - 3; i += 3) // Advance in steps of 3 (cubic segments)
	{
		FVector P0 = path[i];
		FVector P1 = path[i + 1];
		FVector P2 = path[i + 2];
		FVector P3 = path[i + 3];

		// Interpolate the cubic Bezier curve
		for (int32 j = 0; j <= SegmentPerPath; ++j)
		{
			float t = (float)j / (float)SegmentPerPath;

			FVector Point =
				FMath::Pow(1 - t, 3) * P0 +
				3 * FMath::Pow(1 - t, 2) * t * P1 +
				3 * (1 - t) * FMath::Pow(t, 2) * P2 +
				FMath::Pow(t, 3) * P3;

			BezierPath.Add(Point);
		}
	}

	if (!BezierPath.Contains(path.Last()))
	{
		BezierPath.Add(path.Last());
	}


	return BezierPath;

}


void Heapify(TArray<FAStarNodeData>& list, int i)
{
	int parent = (i - 1) / 2;
	if (parent > -1)
	{
		if (list[i] < list[parent])
		{
			FAStarNodeData pom = list[i];
			list[i] = list[parent];
			list[parent] = pom;
			Heapify(list, parent);
		}
	}
}

 void HeapifyDeletion(TArray<FAStarNodeData>& list, int i)
{
	int smallest = i;
	int l = 2 * i + 1;
	int r = 2 * i + 2;

	if (l < list.Num() && list[l] < list[smallest])
	{
		smallest = l;
	}
	if (r < list.Num() && list[r] < list[smallest])
	{
		smallest = r;
	}
	if (smallest != i)
	{
		FAStarNodeData pom = list[i];
		list[i] = list[smallest];
		list[smallest] = pom;

		// Recursively heapify the affected sub-tree
		HeapifyDeletion(list, smallest);
	}
}

void UAStarAgentComponent::MoveTo(FVector Goal)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(MoveTo);
	if (!m_Agent) {
		UE_LOG(LogTemp, Error, TEXT("Invalid Agent Reference"));
		return;
	}

	if (!m_PathGrid) {
		UE_LOG(LogTemp, Error, TEXT("Invalid Path Grid Ref"));
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("Start a new path"));
	FNodeRealData* StartNode = m_PathGrid->GetClosestNode(GetOwner()->GetActorLocation());
	FNodeRealData* GoalNode = m_PathGrid->GetClosestNode(Goal);

	if (m_Path.Num() > 0) {
		/*Clean out all of the path occupied by agent*/
		for (const FVector& location : m_Path) {
			if (auto node = m_PathGrid->GetNode(m_PathGrid->ConvertLocationToPoint(location))) {
				node->RemoveOccupiingAgent(GetOwner());
			}
		}

	}

	m_Path.Empty();

	SetPathfindingState(EPathfindingStatus::InProgress);

	if (StartNode == GoalNode || StartNode == nullptr || GoalNode == nullptr) {
		SetPathfindingState(EPathfindingStatus::Failed);
		UE_LOG(LogTemp, Error, TEXT("failed to move, start node == goal node or Both node invalid"));
		return;
	}

	if (EnablePathFindingDebug) {
		DrawDebugSphere(GetWorld(), StartNode->Location, 300.0f, 4.0f, FColor::Yellow, false, 10.0f, 0, 2);
		DrawDebugSphere(GetWorld(), GoalNode->Location, 300.0f, 4.0f, FColor::Green, false, 10.0f, 0, 2);
	}
	GetWorld()->GetTimerManager().ClearTimer(m_PathFindingHandle);

	/*Move heavy compute itteration to background thread to make performance better*/
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, StartNode, GoalNode]()
		{

			TArray<FAStarNodeData> OpenList;
			TArray<TArray<TArray<FAStarNodeData>>> ClosedList;
			TSet<FIntVector> ClosedCoord;

			FIntVector GridSize = m_PathGrid->SpacingBetweenNode;

			ClosedList.SetNum(GridSize.X);
			for (int i = 0; i < GridSize.X; i++) {
				ClosedList[i].SetNum(GridSize.Y);
				for (int j = 0; j < GridSize.Y; j++) {
					ClosedList[i][j].SetNum(GridSize.Z);

				}

			}

			FAStarNodeData CurrentNodeData(StartNode);

			CurrentNodeData.gCost = 0;
			CurrentNodeData.hCost = FVector::Dist(StartNode->Location, GoalNode->Location);
			CurrentNodeData.TimeToReach = FVector::Dist(StartNode->Location, GoalNode->Location) / FlyingSpeed;

			OpenList.Add(CurrentNodeData);

			ClosedList[StartNode->Coord.X][StartNode->Coord.Y][StartNode->Coord.Z] = CurrentNodeData;


			int loopNumber = 0;

			float TotalDistance = FVector::Dist(GetOwner()->GetActorLocation(), CurrentNodeData.GetLocation());

			while (OpenList.Num() > 0) {

				loopNumber++;

				FAStarNodeData Current;

				Current = OpenList.IsValidIndex(0) ? OpenList[0] : FAStarNodeData();

				if (Current.Node->Coord == GoalNode->Coord)
				{
					//[](const FAStarNodeData& A, const FAStarNodeData& B) { return A < B; }
					m_Path = ReconstructPath(*GoalNode, ClosedList);

					AsyncTask(ENamedThreads::GameThread, [this, &loopNumber]()
						{
							DrawDebugPath(m_Path);
							AgentMove();
							UE_LOG(LogTemp, Error, TEXT("Loop for flying pathfinding: %i"), loopNumber);
							UE_LOG(LogTemp, Error, TEXT("Found Path Number: %i"), m_Path.Num());

						});
					UE_LOG(LogTemp, Error, TEXT("oi"));
					return;
				}

				OpenList.RemoveAt(0);
				HeapifyDeletion(OpenList, 0);
			//	ClosedList[CurrentNodeData.Node->Coord.X][CurrentNodeData.Node->Coord.Y][CurrentNodeData.Node->Coord.Z] = Current;

				if (Current.IsValidNode()) {
					//UE_LOG(LogTemp, Error, TEXT("Neighbour Number %i"), Current.GetNeighbour().Num());
					/*Change thing over here, fix thing*/
					for (int i = 0; i < Current.GetNeighbour().Num(); i++)
					{
						FIntVector NeighborIndex = Current.GetNeighbour()[i];
						FNodeRealData* Neighbor = m_PathGrid->GetNode(NeighborIndex);
						FAStarNodeData NeighborClosedData = ClosedList[NeighborIndex.X][NeighborIndex.Y][NeighborIndex.Z];

						bool NeighbourPassThrough = true;;

						if (!NeighborClosedData.Node) {
							NeighbourPassThrough = false;
							
						}

						if (Neighbor == nullptr)
							continue;
						
						if (Neighbor->CheckIsNodeInvalid())
							continue;
						//
						float arrivalTime = TotalDistance / FlyingSpeed;
						///*Check if any agent will pass through this path*/
						if (OpenList.Num() > 0 && Neighbor->CheckIfOccupiedByAgent(GetOwner(), arrivalTime)) {
							UE_LOG(LogTemp, Error, TEXT("Skip because occupied by other agent"));
							continue;
						}
						//
						//if (ClosedCoord.Contains(NeighborIndex))
						//{
						//	continue;
						//}

						if (Neighbor == GoalNode) {

							if (Neighbor->CheckIsNodeInvalid()) {
								AsyncTask(ENamedThreads::GameThread, [this]()
									{
										SetPathfindingState(EPathfindingStatus::Failed);
										UE_LOG(LogTemp, Error, TEXT("Invalid Path"));
									});
								return;
							}
						}

						FAStarNodeData NeighbourData(Neighbor);

						///*Make sure straight beautiful path is in consideration*/
						float DirectionChangePenalty = 0.0f;
						if (Current.CameFrom.X != -1)
						{
							FVector PreviousDirection = (Current.GetLocation() - Neighbor->Location).GetSafeNormal();
							FVector CurrentDirection = (NeighbourData.GetLocation() - Current.GetLocation()).GetSafeNormal();
						
							// Calculate the deviation angle (dot product gives cosine of the angle)
							float Deviation = FVector::DotProduct(PreviousDirection, CurrentDirection);
							DirectionChangePenalty = (1.0f - Deviation) * PenaltyWeight; // PenaltyWeight is a configurable parameter
						}


						/*Calculate new GCost*/
						float Distance = FVector::Dist(Current.GetLocation(), NeighbourData.GetLocation());
						float NewGCost = Current.gCost + Distance + DirectionChangePenalty;

						if (NewGCost < NeighbourData.gCost)
						{
							NeighbourData.gCost = NewGCost;
							NeighbourData.hCost = FVector::Dist(NeighbourData.GetLocation(), GoalNode->Location);
							NeighbourData.CameFrom = Current.Node->Coord;
							NeighbourData.TimeToReach = Current.TimeToReach + Distance / FlyingSpeed;

							if (!NeighbourPassThrough)
							{
								ClosedList[NeighborIndex.X][NeighborIndex.Y][NeighborIndex.Z] = NeighbourData;
								OpenList.Add(NeighbourData);
								//OpenList.Sort([](const FAStarNodeData& A, const FAStarNodeData& B) { return A < B; });

								Heapify(OpenList, OpenList.Num() - 1);
							}
						}
					}
				}
				else {
					UE_LOG(LogTemp, Error, TEXT("Invalid Node, cannot continue loop"));
				}
			}

			AsyncTask(ENamedThreads::GameThread, [this]()
				{
					SetPathfindingState(EPathfindingStatus::Failed);
					UE_LOG(LogTemp, Error, TEXT("Invalid Path"));
				});

		});

}

EPathfindingStatus UAStarAgentComponent::GetAgentStatus() const
{
	return m_AgentStatus;
}

AAStarPathGrid* UAStarAgentComponent::GetGridReference() const
{
	return m_PathGrid;
}

FVector UAStarAgentComponent::GetCurrentMovingLocation() const
{
	return m_Path.Num() > 0 ? m_Path[m_Path.Num() - 1] : FVector(0);
}


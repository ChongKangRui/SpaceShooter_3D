// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_FlyTo.h"
#include "AIController.h"
#include "Pathfinding/AStarAgentComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"

UBTTask_FlyTo::UBTTask_FlyTo()
{
	bCreateNodeInstance = true;
}


EBTNodeResult::Type UBTTask_FlyTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {

	// Get AI Controller
	m_AIController = OwnerComp.GetAIOwner();
	if (!m_AIController || !m_AIController->GetBlackboardComponent())
		return EBTNodeResult::Failed;

	// Get AI Pawn
	APawn* AIPawn = m_AIController->GetPawn();
	if (!AIPawn) return EBTNodeResult::Failed;

	if (FlyingMode == EFlyingMode::FlyToTarget || FlyingMode == EFlyingMode::FlyAroundTarget) {
		auto target = m_AIController->GetBlackboardComponent()->GetValueAsObject(Target.SelectedKeyName);
		if (target) {
			m_Target = Cast<AActor>(target);

			if (!m_Target)
				return EBTNodeResult::Failed;
		}
	}


	// Attempt to get the flying component
	if (UAStarAgentComponent* AgentComp = AIPawn->FindComponentByClass<UAStarAgentComponent>())
	{
		m_Agent = AgentComp;
		m_Agent->OnAgentMoving.AddDynamic(this, &UBTTask_FlyTo::OnAgentMoving);
		m_Agent->OnPathfindingStateChanged.AddDynamic(this, &UBTTask_FlyTo::OnAgentStateChanged);

		m_AgentDefaultsAcceptableRange = m_Agent->AcceptableRange;
		m_Agent->AcceptableRange = AcceptableRadius;

		if (FlyingMode == EFlyingMode::FlyToTarget) {

			m_Agent->MoveTo(m_Target->GetActorLocation());
		}
		else if (FlyingMode == EFlyingMode::FlyAroundTarget) {
			m_Agent->MoveTo(m_Agent->GetGridReference()->GetRandomLocationWithinRange(m_Target->GetActorLocation(), MinimumRadius, MaximumRadius));
		}
		else {
			m_Agent->MoveTo(m_Agent->GetGridReference()->GetRandomLocationWithinRange(m_Agent->GetOwner()->GetActorLocation(), MinimumRadius, MaximumRadius));
		}

		return EBTNodeResult::InProgress;  // Return InProgress if it's async
	}

	return EBTNodeResult::Failed;
}

void UBTTask_FlyTo::UpdateTargetLocation()
{
}

void UBTTask_FlyTo::OnAgentMoving(FVector NextPosition)
{
	/*Inside acceptable radius, end the pathfinding*/
	
	FRotator TargetRotation;
	FVector DirectionToTarget;

	FVector AgentLocation = m_Agent->GetOwner()->GetActorLocation();

	if (RotateTowardTarget) {
		RotateToTarget(NextPosition);
	}
	else {
		RotateTowardNextWayPoint(NextPosition);
	}


	if (FlyingMode == EFlyingMode::FlyToTarget) {
		if (m_Target) {
			/*Keep refresh location*/
			/*Is this the problem that keep reloop every thing?*/
			UE_LOG(LogTemp, Error, TEXT("Distancechecl: %f"), FVector::Dist(m_Agent->GetCurrentMovingLocation(), m_Target->GetActorLocation()));
			//if (FVector::Dist(m_Agent->GetCurrentMovingLocation(), m_Target->GetActorLocation()) > DistanceToKeepTrack) {
			//
			//	m_Agent->MoveTo(m_Target->GetActorLocation());
			//
			//}
		}
	}
}

void UBTTask_FlyTo::OnAgentStateChanged(EPathfindingStatus newStatus)
{
	if (newStatus == EPathfindingStatus::Success || newStatus == EPathfindingStatus::Failed) {

		m_Agent->AcceptableRange = m_AgentDefaultsAcceptableRange;
		m_Agent->OnAgentMoving.RemoveDynamic(this, &UBTTask_FlyTo::OnAgentMoving);
		m_Agent->OnPathfindingStateChanged.RemoveDynamic(this, &UBTTask_FlyTo::OnAgentStateChanged);

		if (UBehaviorTreeComponent* OwnerComp = Cast<UBehaviorTreeComponent>(m_AIController->GetBrainComponent()))
		{

			FinishLatentTask(*OwnerComp, newStatus == EPathfindingStatus::Success ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
		}


	}

}

void UBTTask_FlyTo::RotateToTarget(FVector nextWayPoint)
{
	FRotator TargetRotation;
	FVector DirectionToTarget;

	FVector AgentLocation = m_Agent->GetOwner()->GetActorLocation();

	if (RotateTowardTarget) {
		FVector DirectionToNextPosition = (nextWayPoint - AgentLocation).GetSafeNormal();

		FVector DirectionTowardTarget = (m_Target->GetActorLocation() - AgentLocation).GetSafeNormal();

		if (FVector::DotProduct(DirectionToNextPosition, DirectionTowardTarget) >= ThresholdRotateToTarget) {
			DirectionToTarget = DirectionTowardTarget;
		}
		else {
			RotateTowardNextWayPoint(nextWayPoint);
			return;
		}
		TargetRotation = DirectionToTarget.Rotation();
	}


	FRotator AgentNewRotation = FMath::RInterpTo(m_Agent->GetOwner()->GetActorRotation(),
		TargetRotation, GetWorld()->GetDeltaSeconds(), RotateSpeed);

	m_Agent->GetOwner()->SetActorRotation(AgentNewRotation);
}

void UBTTask_FlyTo::RotateTowardNextWayPoint(FVector nextWayPoint)
{
	const FVector& DirectionToTarget = nextWayPoint - m_Agent->GetOwner()->GetActorLocation();
	const FVector& DirectionToTargetNormalize = DirectionToTarget.GetSafeNormal();

	float YawRadian = FMath::Atan2(DirectionToTargetNormalize.Y, DirectionToTargetNormalize.X);
	float YawAngle = FMath::RadiansToDegrees(YawRadian);

	float PitchRadian = FMath::Atan2(DirectionToTargetNormalize.Z, FVector(DirectionToTargetNormalize.X, DirectionToTargetNormalize.Y, 0).Size());
	float PitchAngle = FMath::RadiansToDegrees(PitchRadian);

	FRotator TargetRotation = FRotator(PitchAngle, YawAngle, 0);

	FRotator AgentNewRotation = FMath::RInterpTo(m_Agent->GetOwner()->GetActorRotation(),
	TargetRotation, GetWorld()->GetDeltaSeconds(), RotateSpeed);

	m_Agent->GetOwner()->SetActorRotation(AgentNewRotation);
}

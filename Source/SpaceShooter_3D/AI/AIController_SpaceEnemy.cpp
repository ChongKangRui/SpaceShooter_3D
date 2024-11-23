// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AIController_SpaceEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"


void AAIController_SpaceEnemy::SetTargetAsPlayer()
{
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        if (auto pc = GetWorld()->GetFirstPlayerController()) {
            // Set an Object Value (like another Actor)
            AActor* TargetActor = pc->GetPawn();

            if (TargetActor)
                BlackboardComp->SetValueAsObject(TEXT("Target"), TargetActor);
            else
                UE_LOG(LogTemp, Error, TEXT("Invalid Target Player"));
        }

    }
}

void AAIController_SpaceEnemy::SetTargetFromTag()
{
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        TArray<AActor*> foundActors;
        UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AActor::StaticClass(), EnemyTag, foundActors);

        if (foundActors.Num() <= 0) {
            UE_LOG(LogTemp, Error, TEXT("Cant found related enemy with tag"));
            return;
        }

        int randomInt = FMath::RandRange(0, foundActors.Num() - 1);

        BlackboardComp->SetValueAsObject(TEXT("Target"), foundActors[randomInt]);

    }
}

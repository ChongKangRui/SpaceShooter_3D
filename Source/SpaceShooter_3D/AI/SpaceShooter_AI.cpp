// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceShooter_AI.h"
#include "AI/AIController_SpaceEnemy.h"

void ASpaceShooter_AI::PossessedBy(AController* NewController)
{
	if (auto AIController = Cast<AAIController_SpaceEnemy>(NewController)) {
		Super::PossessedBy(NewController);
		m_AIController = AIController;
	}
}

void ASpaceShooter_AI::OnTakeAnyDamageBinding(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (!m_AIController) {
		UE_LOG(LogTemp, Error, TEXT("Error: %s 's AIController not initialize."), *GetName());
		return;
	}

	Super::OnTakeAnyDamageBinding(DamagedActor,Damage,  DamageType,  InstigatedBy, DamageCauser);

	if (!m_AIController->HaveTarget()) {
		if (auto pc = GetWorld()->GetFirstPlayerController()) {
			// Set an Object Value (like another Actor)
			AActor* Player = pc->GetPawn();
			if (Player == DamageCauser) {
				m_AIController->SetTargetAsPlayer();
			}
			else {
				//Set Other Actor As Enemy
			}

		}
	}
}

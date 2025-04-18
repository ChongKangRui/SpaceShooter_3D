// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_Shoot.h"
#include "AI/AIController_SpaceEnemy.h"
#include "Character/SpaceShooter_3DCharacter.h"


UBTService_Shoot::UBTService_Shoot()
{
	bCreateNodeInstance = true;
}

void UBTService_Shoot::OnSearchStart(FBehaviorTreeSearchData& SearchData)
{
	Super::OnSearchStart(SearchData);
	m_HeavyWeaponCD = FMath::RandRange(FrequencyOfHeavyWeaponAttack.X, FrequencyOfHeavyWeaponAttack.Y);
}

void UBTService_Shoot::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (!m_OwnerController || !m_OwnerCharacter) {
		m_OwnerController = Cast<AAIController_SpaceEnemy>(OwnerComp.GetAIOwner());

		if (m_OwnerController) {
			m_OwnerCharacter = Cast<ASpaceShooter_3DCharacter>(m_OwnerController->GetPawn());
		}

	}
	else {

		if (!m_OwnerCharacter) {
			UE_LOG(LogTemp, Error, TEXT("UBTService_Shoot: Invalid owner character, unable to shoot"));
			return;
		}

		m_CurrentHWTimer += DeltaSeconds;
		m_CurrentHWShootDuration += DeltaSeconds;

		FVector OwnerLoc = m_OwnerCharacter->GetActorLocation();
		FVector TargetLoc = m_OwnerController->GetTarget()->GetActorLocation();

		FVector NormalDir = (TargetLoc - OwnerLoc).GetSafeNormal();

		float DotProductValue = FVector::DotProduct(NormalDir, m_OwnerController->GetPawn()->GetActorForwardVector());
		float AngleInRadians = FMath::Acos(DotProductValue);
		float Degree = FMath::RadiansToDegrees(AngleInRadians);

		

		if (FMath::Abs(Degree) <= (AngleToShoot / 2)) {

			//m_OwnerCharacter->StartFireMissle(EWeaponType::Light);
			if(!DisableNormalAttack)
				m_OwnerCharacter->FireWeapon(EWeaponType::Light);

			if (m_CurrentHWTimer >= m_HeavyWeaponCD && !m_OwnerCharacter->GetIsWeaponLaser(EWeaponType::Heavy)) {
				ShootHeavyWeapon();
			}
			else {
				m_ShootLaser = true;
			}
		}
		else {
			if (!m_OwnerCharacter->GetIsWeaponLaser(EWeaponType::Heavy)) {
				m_OwnerCharacter->StopWeapon(EWeaponType::Heavy);
				UE_LOG(LogTemp, Error, TEXT("stop laser"));
			}
			
		}

		if (m_ShootLaser) {
			ShootHeavyWeapon();

			
		}

	}
	
}

void UBTService_Shoot::ShootHeavyWeapon()
{
	//m_OwnerCharacter->StartFireMissle(EWeaponType::Heavy);
	m_OwnerCharacter->FireWeapon(EWeaponType::Heavy);

	if (m_CurrentHWShootDuration > m_HeavyWeaponShootDuration) {
		m_CurrentHWTimer = 0;
		m_CurrentHWShootDuration = 0;

		m_HeavyWeaponCD = FMath::RandRange(FrequencyOfHeavyWeaponAttack.X, FrequencyOfHeavyWeaponAttack.Y);
		m_HeavyWeaponShootDuration = FMath::RandRange(DurationOfHeavyWeaponShooting.X, DurationOfHeavyWeaponShooting.Y);

		m_ShootLaser = false;
	}
}

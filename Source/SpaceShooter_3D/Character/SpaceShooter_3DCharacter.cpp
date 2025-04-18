// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceShooter_3DCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Pathfinding/AStarAgentComponent.h"
#include "Ship/ShipProjectile.h"
#include "Ship/LaserBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "InputActionValue.h"
#include "SpaceShooter_3DGameMode.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "SpaceShooterGameInstance.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ASpaceShooter_3DCharacter

ASpaceShooter_3DCharacter::ASpaceShooter_3DCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(51.481384, 51.481384);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->MaxFlySpeed = 600.0;
	GetCharacterMovement()->BrakingDecelerationFlying = 150.0f;
	GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Flying;

	AgentComponent = CreateDefaultSubobject<UAStarAgentComponent>(TEXT("AStar Agent"));

	ShipChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("Ship"));
	ShipChildActor->SetupAttachment(GetMesh());

	ShipChildActor->SetChildActorClass(AActor::StaticClass());

	//DataTableAssetInitialization();
}

const FVector ASpaceShooter_3DCharacter::GetShootLocation() const
{
	return GetActorLocation();
}

const FVector ASpaceShooter_3DCharacter::GetShootDirection() const
{
	return GetActorForwardVector();
}

const FVector ASpaceShooter_3DCharacter::GetRandomShootOffset() const
{
	return FVector();
}

AActor* ASpaceShooter_3DCharacter::GetMissleTrackEnemy() const
{
	return nullptr;
}

void ASpaceShooter_3DCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (bRandomizeMesh) {
		int32 EnumMin = 0;
		int32 EnumMax = static_cast<int32>(EShipType::SpaceShip3);

		int32 RandomEnumValue = FMath::RandRange(EnumMin, EnumMax);

		EShipType type = static_cast<EShipType>(RandomEnumValue);
		SetShip(type);
	}
	else
		SetShip(ShipType);

	if (const AActor* Ship = ShipChildActor->GetChildActor()) {
		for (UActorComponent* ac : Ship->GetComponentsByTag(USceneComponent::StaticClass(), "Light")) {
			if (USceneComponent* sc = Cast<USceneComponent>(ac)) {
				m_LightWeaponFirePoint.Add(sc);
				
			}
		}
		for (UActorComponent* ac : Ship->GetComponentsByTag(USceneComponent::StaticClass(), "Heavy")) {
			if (USceneComponent* sc = Cast<USceneComponent>(ac)) {
				m_HeavyWeaponFirePoint.Add(sc);
				
			}
		}

		RelativeRotation_LFP = m_LightWeaponFirePoint[m_LightWeaponFirePoint.Num() - 1]->GetRelativeRotation();
		RelativeRotation_HFP = m_HeavyWeaponFirePoint[m_HeavyWeaponFirePoint.Num() - 1]->GetRelativeRotation();
	}

	m_CurrentHealth = m_ShipAttribute.MaxHealth;

	m_AllowToFireMissle.Add(EWeaponType::Light, true);
	m_AllowToFireMissle.Add(EWeaponType::Heavy, true);

	m_CurrentWeaponAmmunitionAmount.Add(EWeaponType::Light, m_ShipAttribute.LightWeapon.AmmunitionAmount);
	m_CurrentWeaponAmmunitionAmount.Add(EWeaponType::Heavy, m_ShipAttribute.HeavyWeapon.AmmunitionAmount);

	OnTakeAnyDamage.AddDynamic(this, &ASpaceShooter_3DCharacter::OnTakeAnyDamageBinding);
}

void ASpaceShooter_3DCharacter::Tick(float delta)
{
	Super::Tick(delta);
	if (bDebugHeavyWeapon) {

		FireWeapon(EWeaponType::Heavy);


	}
}

//void ASpaceShooter_3DCharacter::DataTableAssetInitialization()
//{
//	//Get datatable
//	static ConstructorHelpers::FObjectFinder<UDataTable> dataTablePath(TEXT("/Script/Engine.DataTable'/Game/Data/DT_ShipAttribute.DT_ShipAttribute'"));
//	m_DataTableAsset = dataTablePath.Object;
//}



//void ASpaceShooter_3DCharacter::StartFireMissle(const EWeaponType WeaponSlot)
//{
//	//StopFireMissle(ArmorSlot);
//
//
//	if (!CanShoot(WeaponSlot))
//		return;
//
//	FireMissle(WeaponSlot);
//
//	m_AllowToFireMissle[WeaponSlot] = false;
//	m_CurrentWeaponAmmunitionAmount[WeaponSlot]--;
//
//	//UE_LOG(LogTemp, Error, TEXT("Remain Armor %i"), m_CurrentWeaponArmorAmount[ArmorSlot]);
//
//	StartWeaponTimer(WeaponSlot);
//}

void ASpaceShooter_3DCharacter::FireLaser(const EWeaponType WeaponSlot)
{
	const TArray<USceneComponent*>& Arr = WeaponSlot == EWeaponType::Light ? m_LightWeaponFirePoint : m_HeavyWeaponFirePoint;
	const FShipWeapon& weaponInfo = WeaponSlot == EWeaponType::Heavy ? m_ShipAttribute.HeavyWeapon : m_ShipAttribute.LightWeapon;

	if (Arr.Num() == 0) {
		UE_LOG(LogTemp, Error, TEXT("Armor Firing failed, no SceneComponent reference in Array"));
		return;
	}

	if (!weaponInfo.ProjectileClass) {
		UE_LOG(LogTemp, Error, TEXT("Invalid Projectile Class"));
		return;
	}

	FHitResult OutHit;
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(FireMissle), /*bTraceComplex=*/ true, /*IgnoreActor=*/ this);
	const ECollisionChannel TraceChannel = ECC_Pawn;

	if (m_LaserInstance.Num() > 0) {

		for (int i = 0; i < m_LaserInstance.Num(); i ++) {
			//FVector Direction = bHit ? (OutHit.Location - laser->GetActorLocation()).GetSafeNormal() : (OutHit.TraceEnd - laser->GetActorLocation()).GetSafeNormal();
			auto laser = m_LaserInstance[i];
			auto sc = Arr[i];
			
			

			FVector TraceStartLocation = laser->GetActorLocation();
			FVector TraceEndLocation = TraceStartLocation + (sc->GetComponentRotation().Vector() * weaponInfo.TraceDistance);

			bool bHit = GetWorld()->SweepSingleByProfile(
				OutHit,
				TraceStartLocation,
				TraceEndLocation,
				FQuat::Identity,
				FName("Pawn"),
				FCollisionShape::MakeSphere(weaponInfo.TraceRadius),
				TraceParams
			);

	

			float Length = bHit ? FVector::Dist(OutHit.Location, laser->GetActorLocation()) : FVector::Dist(OutHit.TraceEnd, laser->GetActorLocation());

			
			//laser->SetActorRotation(Rotation);
			laser->SetLaserLength(FMath::Clamp(Length, 0, weaponInfo.TraceDistance));
			if (bHit) {
				UE_LOG(LogTemp, Error, TEXT("Hit %s"), *OutHit.GetActor()->GetName());
				if (weaponInfo.HitEffect)
				{
					if (m_LaserVFXInstance.IsValid()) {
						m_LaserVFXInstance->SetWorldLocation(OutHit.GetActor()->GetActorLocation());
					}
					else {
						// Spawn the Niagara system
						m_LaserVFXInstance = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), weaponInfo.HitEffect, OutHit.GetActor()->GetActorLocation(), FRotator::ZeroRotator);
					}
				}

				if (OutHit.GetActor()) {
					UGameplayStatics::ApplyDamage(OutHit.GetActor(), weaponInfo.Shoot_Damage, GetInstigatorController(), this, nullptr);
					UE_LOG(LogTemp, Error, TEXT("Damage Apply %f"), Length);
				}

			}
			else {
				if (m_LaserVFXInstance.IsValid())
					m_LaserVFXInstance->DestroyInstance();
			}

		}
		return;

	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Optionally set the owner and instigator
	SpawnParams.Owner = ShipChildActor->GetChildActor();
	SpawnParams.Instigator = this;

	for (USceneComponent* sc : Arr) {
		//Spawn Missle
		if (sc) {

			FVector ShootDirection = GetShootDirection();
			FRotator TargetRotation = GetShootDirection().Rotation();
			/*UE_LOG(LogTemp, Error, TEXT("Init SC Rotation %s"), *TargetRotation.ToString());
			sc->SetWorldRotation(TargetRotation);
		*/
			FVector TraceStartLocation = ShootDirection;
			FVector TraceEndLocation = TraceStartLocation + (ShootDirection * weaponInfo.TraceDistance);

			bool bHit = GetWorld()->SweepSingleByProfile(
				OutHit,
				TraceStartLocation,
				TraceEndLocation,
				FQuat::Identity,
				FName("ShipCollision"),
				FCollisionShape::MakeSphere(weaponInfo.TraceRadius),
				TraceParams
			);

			if (OutHit.GetActor()) {
				UGameplayStatics::ApplyDamage(OutHit.GetActor(), weaponInfo.Shoot_Damage, GetInstigatorController(), GetOwner(), nullptr);
			}

			FVector Direction = bHit ? (OutHit.Location - sc->GetComponentLocation()).GetSafeNormal() : (OutHit.TraceEnd - sc->GetComponentLocation()).GetSafeNormal();
			FRotator SpawnRotation = Direction.Rotation();

			

			// Spawn the actor (replace AMyActor with your actor class)
			ALaserBase* SpawnedActor = GetWorld()->SpawnActor<ALaserBase>(weaponInfo.ProjectileClass, sc->GetComponentLocation(), sc->GetComponentRotation(), SpawnParams);
			m_LaserInstance.Add(SpawnedActor);

			SpawnedActor->AttachToComponent(sc, FAttachmentTransformRules::KeepRelativeTransform);
			SpawnedActor->SetActorRelativeLocation(FVector::Zero());
			SpawnedActor->SetActorRelativeRotation(FRotator::ZeroRotator);
			
			
			sc->SetUsingAbsoluteRotation(true);
			

			SpawnedActor->InitializeLifeTime(m_CurrentWeaponAmmunitionAmount[WeaponSlot] * weaponInfo.Shoot_CD);
		
		}

	}
}

void ASpaceShooter_3DCharacter::StopWeapon(const EWeaponType WeaponSlot)
{
	const FShipWeapon& weaponInfo = WeaponSlot == EWeaponType::Heavy ? m_ShipAttribute.HeavyWeapon : m_ShipAttribute.LightWeapon;

	if (weaponInfo.bIsLaser) {
		int loop = m_LaserInstance.Num();
		const TArray<USceneComponent*>& Arr = WeaponSlot == EWeaponType::Light ? m_LightWeaponFirePoint : m_HeavyWeaponFirePoint;
	
		for (int i = 0; i < loop;i++) {

			auto laser = m_LaserInstance[i];
			auto sc = Arr[i];
			
			
			laser->Destroy();
			sc->SetUsingAbsoluteRotation(false);

			FRotator rotation = WeaponSlot == EWeaponType::Heavy ? RelativeRotation_HFP : RelativeRotation_LFP;
			sc->SetRelativeRotation(rotation);
		}

		m_LaserInstance.Empty();

		if (m_LaserVFXInstance.IsValid())
			m_LaserVFXInstance->DestroyInstance();
	}

}

void ASpaceShooter_3DCharacter::FireMissle(const EWeaponType WeaponSlot)
{
	const TArray<USceneComponent*>& Arr = WeaponSlot == EWeaponType::Light ? m_LightWeaponFirePoint : m_HeavyWeaponFirePoint;
	const FShipWeapon& weaponInfo = WeaponSlot == EWeaponType::Heavy ? m_ShipAttribute.HeavyWeapon : m_ShipAttribute.LightWeapon;

	if (Arr.Num() == 0) {
		UE_LOG(LogTemp, Error, TEXT("Armor Firing failed, no SceneComponent reference in Array"));
		return;
	}

	if (!weaponInfo.ProjectileClass) {
		UE_LOG(LogTemp, Error, TEXT("Invalid Projectile Class"));
		return;
	}

	FHitResult OutHit;
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(FireMissle), /*bTraceComplex=*/ true, /*IgnoreActor=*/ ShipChildActor->GetChildActor());
	const ECollisionChannel TraceChannel = ECC_GameTraceChannel2;

	FVector TraceStartLocation = GetShootLocation();
	FVector TraceEndLocation = TraceStartLocation + (GetShootDirection() * weaponInfo.TraceDistance);

	bool bHit = GetWorld()->SweepSingleByProfile(
		OutHit,
		TraceStartLocation,
		TraceEndLocation,
		FQuat::Identity,
		FName("ShipCollision"),
		FCollisionShape::MakeSphere(weaponInfo.TraceRadius),
		TraceParams
	);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Optionally set the owner and instigator
	SpawnParams.Owner = ShipChildActor->GetChildActor();
	SpawnParams.Instigator = this;


	for (const USceneComponent* sc : Arr) {
		//Spawn Missle
		if (sc) {
			FVector Direction = bHit ? (OutHit.Location - sc->GetComponentLocation()).GetSafeNormal() : (OutHit.TraceEnd - sc->GetComponentLocation()).GetSafeNormal();
			FRotator SpawnRotation = Direction.Rotation();

			//UE_LOG(LogTemp, Error, TEXT("Shoot Location %s, Hitt? %s"), *SpawnRotation.ToString(), bHit ? *OutHit.GetActor()->GetName() : TEXT("No hit actor"));

			// Spawn the actor (replace AMyActor with your actor class)
			AShipProjectile* SpawnedActor = GetWorld()->SpawnActor<AShipProjectile>(weaponInfo.ProjectileClass, sc->GetComponentLocation(), SpawnRotation, SpawnParams);

			AActor* TrackTarget = GetMissleTrackEnemy() ? GetMissleTrackEnemy() : OutHit.GetActor();


			//UE_LOG(LogTemp, Error, TEXT("IS Track Target Valid? %s"), OutHit.GetActor() ? TEXT("Valid") : TEXT("Invalid"));
			if (SpawnedActor)
				SpawnedActor->Initialization(weaponInfo.Shoot_Damage, weaponInfo.CanTrackEnemy ? TrackTarget : nullptr, weaponInfo.TrackingRotateSpeed, weaponInfo.HitEffect);
		}

	}


}

bool ASpaceShooter_3DCharacter::CanShoot(const EWeaponType WeaponSlot) const
{
	return m_AllowToFireMissle[WeaponSlot];
}

bool ASpaceShooter_3DCharacter::IsAmmoOut(const EWeaponType WeaponSlot) const
{

	return m_CurrentWeaponAmmunitionAmount[WeaponSlot] <= 0;
}

void ASpaceShooter_3DCharacter::StartWeaponTimer(const EWeaponType WeaponSlot)
{
	const FShipWeapon& WeaponAttribute = WeaponSlot == EWeaponType::Heavy ? m_ShipAttribute.HeavyWeapon : m_ShipAttribute.LightWeapon;
	FTimerHandle& currentReloadTimer = WeaponSlot == EWeaponType::Heavy ? m_HeavyWeaponReload_Timer : m_LightWeaponReload_Timer;

	FTimerHandle TempTimer;
	FTimerDelegate ShootDelegate, ReloadDelegate;

	ShootDelegate.BindWeakLambda(this, [&, WeaponSlot]()
		{
			m_AllowToFireMissle[WeaponSlot] = true;
		});
	GetWorld()->GetTimerManager().SetTimer(TempTimer, ShootDelegate, 0.01f, false, WeaponAttribute.Shoot_CD);

	/*Reload*/
	GetWorld()->GetTimerManager().ClearTimer(currentReloadTimer);
	ReloadDelegate.BindWeakLambda(this, [&, WeaponSlot]()
		{
			m_CurrentWeaponAmmunitionAmount[WeaponSlot] = WeaponAttribute.AmmunitionAmount;
		});
	GetWorld()->GetTimerManager().SetTimer(currentReloadTimer, ReloadDelegate, 0.01f, false, m_CurrentWeaponAmmunitionAmount[WeaponSlot] > 0 ? WeaponAttribute.ArmorRefillCD : WeaponAttribute.CoolDownCD);

}

void ASpaceShooter_3DCharacter::UpdateLaserRotation(const EWeaponType WeaponSlot)
{
	if (!GetIsWeaponLaser(WeaponSlot))
		return;

	const TArray<USceneComponent*>& Arr = WeaponSlot == EWeaponType::Light ? m_LightWeaponFirePoint : m_HeavyWeaponFirePoint;
	const FShipWeapon& weaponInfo = WeaponSlot == EWeaponType::Heavy ? m_ShipAttribute.HeavyWeapon : m_ShipAttribute.LightWeapon;

	for (int i = 0; i < m_LaserInstance.Num(); i++) {

		auto sc = Arr[i];

		FVector Direction = GetShootDirection();
		FRotator TargetRotation = GetShootDirection().Rotation();
		FRotator NewRotation = FMath::RInterpConstantTo(sc->GetComponentRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), weaponInfo.TrackingRotateSpeed);

		sc->SetWorldRotation(TargetRotation);

		UE_LOG(LogTemp, Error, TEXT("NewRotation %s, sc world loc %s"), *TargetRotation.ToString(), *sc->GetComponentRotation().ToString())
	}

}

void ASpaceShooter_3DCharacter::OnTakeAnyDamageBinding(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	m_CurrentHealth = FMath::Clamp(m_CurrentHealth - Damage, 0, m_ShipAttribute.MaxHealth);

	if (auto character = Cast<ASpaceShooter_3DCharacter>(DamageCauser)) {
		character->OnBulletHit.Broadcast();
	}


	UE_LOG(LogTemp, Error, TEXT("Health %f"), m_CurrentHealth);
	if (m_CurrentHealth <= 0)
		Destroy();
}

void ASpaceShooter_3DCharacter::SwitchWeapon(const EWeaponType ArmorSlot)
{
	//int MaxWeapon = ArmorSlot == EWeaponType::LightArmor ? m_ShipAttribute.LightArmor.Num() - 1 : m_ShipAttribute.HeavyArmor.Num() - 1;
	//
	//int CurrentWeapon = m_CurrentWeapon[ArmorSlot];
	//
	//if (MaxWeapon <= 0) {
	//	return;
	//}
	//
	//if (CurrentWeapon < MaxWeapon) {
	//	CurrentWeapon++;
	//}
	//else {
	//	CurrentWeapon = 0;
	//}
	//
	//m_CurrentWeapon.Add(ArmorSlot, CurrentWeapon);
	///*Reset the timer so it can work with correct shooting rate*/
	//ResetShootingTimer(ArmorSlot);
}

void ASpaceShooter_3DCharacter::SetShip(const EShipType& ShipToUse)
{
	EShipType finalShipType = ShipToUse;
	
	if (const auto GM = GetWorld()->GetAuthGameMode<ASpaceShooter_3DGameMode>()) {
		if (IsPlayer) {

			finalShipType = GM->GetGameInstance<USpaceShooterGameInstance>()->GetPlayerShipType();
			
		}
		m_ShipAttribute = GM->GetAttribute(finalShipType);

		if (m_ShipAttribute.ShipMesh) {
			// Spawn the actor from the class
			ShipChildActor->SetChildActorClass(m_ShipAttribute.ShipMesh);
			UE_LOG(LogTemp, Error, TEXT("valid Ship Data"));
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("Invalid Ship Data"));
		}
	}
}


bool ASpaceShooter_3DCharacter::GetIsWeaponLaser(EWeaponType WeaponSlot) const
{
	const FShipWeapon& weaponInfo = WeaponSlot == EWeaponType::Heavy ? m_ShipAttribute.HeavyWeapon : m_ShipAttribute.LightWeapon;
	return weaponInfo.bIsLaser;
}

UPaperSprite* ASpaceShooter_3DCharacter::GetWeaponIcon(EWeaponType WeaponSlot) const
{
	const FShipWeapon& weaponInfo = WeaponSlot == EWeaponType::Heavy ? m_ShipAttribute.HeavyWeapon : m_ShipAttribute.LightWeapon;
	return weaponInfo.WeaponIcon;
}

float ASpaceShooter_3DCharacter::GetMaxHealth() const
{
	return  m_ShipAttribute.MaxHealth;
}

float ASpaceShooter_3DCharacter::GetCurrentHealth() const
{
	return m_CurrentHealth;
}

float ASpaceShooter_3DCharacter::GetCurrentWeaponAmmunition(EWeaponType weaponType) const
{

	return m_CurrentWeaponAmmunitionAmount[weaponType];
}

float ASpaceShooter_3DCharacter::GetMaxWeaponAmmunition(EWeaponType weaponType) const
{
	const FShipWeapon& weaponInfo = weaponType == EWeaponType::Heavy ? m_ShipAttribute.HeavyWeapon : m_ShipAttribute.LightWeapon;
	return weaponInfo.AmmunitionAmount;

}

bool ASpaceShooter_3DCharacter::IsInitialized() const
{
	return m_ShipAttribute.ShipMesh ? true : false;
}

void ASpaceShooter_3DCharacter::FireWeapon(const EWeaponType WeaponSlot)
{
	UpdateLaserRotation(WeaponSlot);
	if (!CanShoot(WeaponSlot)) {

		return;
	}
	const FShipWeapon& weaponInfo = WeaponSlot == EWeaponType::Heavy ? m_ShipAttribute.HeavyWeapon : m_ShipAttribute.LightWeapon;
	if (IsAmmoOut(WeaponSlot)) {
		StopWeapon(WeaponSlot);
		//UE_LOG(LogTemp, Error, TEXT("Out of ammo"));
		return;
	}

	if (weaponInfo.bIsLaser)
		FireLaser(WeaponSlot);
	else
		FireMissle(WeaponSlot);

	m_AllowToFireMissle[WeaponSlot] = false;
	m_CurrentWeaponAmmunitionAmount[WeaponSlot]--;

	StartWeaponTimer(WeaponSlot);
}




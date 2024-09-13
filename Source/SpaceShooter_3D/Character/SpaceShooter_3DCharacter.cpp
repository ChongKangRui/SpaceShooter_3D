// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceShooter_3DCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Pathfinding/AStarAgentComponent.h"
#include "Ship/ShipProjectile.h"
#include "InputActionValue.h"

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

	DataTableAssetInitialization();
}

const FVector ASpaceShooter_3DCharacter::GetShootLocation() const
{
	return GetActorLocation();
}

const FVector ASpaceShooter_3DCharacter::GetShootDirection() const
{
	return GetActorForwardVector();
}

void ASpaceShooter_3DCharacter::BeginPlay()
{
	Super::BeginPlay();

	m_CurrentWeapon.Add(EWeaponType::LightArmor, 0);
	m_CurrentWeapon.Add(EWeaponType::HeavyArmor, 0);

	if (bRandomizeMesh) {
		int32 EnumMin = 0;
		int32 EnumMax = static_cast<int32>(EShipType::SpaceShip3);

		int32 RandomEnumValue = FMath::RandRange(EnumMin, EnumMax);

		EShipType type = static_cast<EShipType>(RandomEnumValue);
		SetShip(type);
	}
	else
		SetShip(ShipMeshType);

	if (const AActor* Ship = ShipChildActor->GetChildActor()) {
		for (UActorComponent* ac : Ship->GetComponentsByTag(USceneComponent::StaticClass(), "Light")) {
			if (USceneComponent* sc = Cast<USceneComponent>(ac)) {
				m_LightArmorFirePoint.Add(sc);
			}
		}
		for (UActorComponent* ac : Ship->GetComponentsByTag(USceneComponent::StaticClass(), "Heavy")) {
			if (USceneComponent* sc = Cast<USceneComponent>(ac)) {
				m_HeavyArmorFirePoint.Add(sc);
			}
		}

	}
}

void ASpaceShooter_3DCharacter::DataTableAssetInitialization()
{
	//Get datatable
	static ConstructorHelpers::FObjectFinder<UDataTable> dataTablePath(TEXT("/Script/Engine.DataTable'/Game/Data/DT_ShipAttribute.DT_ShipAttribute'"));
	m_DataTableAsset = dataTablePath.Object;
}

void ASpaceShooter_3DCharacter::ResetShootingTimer(const EWeaponType ArmorSlot)
{
	FTimerHandle& currentTimer = ArmorSlot == HeavyArmor ? m_HeavyArmor_Timer : m_LightArmor_Timer;
	if (GetWorld()->GetTimerManager().IsTimerActive(currentTimer)) {
		StartFireMissle(ArmorSlot);
	}
}

void ASpaceShooter_3DCharacter::StartFireMissle(const EWeaponType ArmorSlot)
{
	StopFireMissle(ArmorSlot);

	FTimerHandle& currentTimer = ArmorSlot == HeavyArmor ? m_HeavyArmor_Timer : m_LightArmor_Timer;
	FTimerDelegate ShootDelegate;

	const TArray<FShipArmor>& WeaponArmorArr = ArmorSlot == HeavyArmor ? m_ShipAttribute.HeavyArmor : m_ShipAttribute.LightArmor;

	if (WeaponArmorArr.IsValidIndex(m_CurrentWeapon[ArmorSlot])) {

		ShootDelegate.BindWeakLambda(this, [&, ArmorSlot]()
			{

				FireMissle(ArmorSlot, m_CurrentWeapon[ArmorSlot]);
			});

		GetWorld()->GetTimerManager().SetTimer(currentTimer, ShootDelegate, WeaponArmorArr[m_CurrentWeapon[ArmorSlot]].Shoot_CD, true);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Invalid Armor Weapon"));
	}

}

void ASpaceShooter_3DCharacter::StopFireMissle(const EWeaponType ArmorSlot)
{
	FTimerHandle& currentTimer = ArmorSlot == HeavyArmor ? m_HeavyArmor_Timer : m_LightArmor_Timer;

	if (GetWorld()->GetTimerManager().IsTimerActive(currentTimer)) {
		GetWorld()->GetTimerManager().ClearTimer(currentTimer);
		currentTimer.Invalidate();
	}
}

void ASpaceShooter_3DCharacter::FireMissle(const EWeaponType ArmorSlot, const int WeaponSlot)
{
	const TArray<USceneComponent*>& Arr = ArmorSlot == LightArmor ? m_LightArmorFirePoint : m_HeavyArmorFirePoint;

	const TArray<FShipArmor>& WeaponArmorArr = ArmorSlot == HeavyArmor ? m_ShipAttribute.HeavyArmor : m_ShipAttribute.LightArmor;

	const FShipArmor& weaponInfo = WeaponArmorArr[WeaponSlot];

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

	bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, TraceStartLocation, TraceEndLocation, TraceChannel, TraceParams);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Optionally set the owner and instigator
	SpawnParams.Owner = this;  // 'this' could be the actor spawning the new one
	SpawnParams.Instigator = this;


	for (const USceneComponent* sc : Arr) {
		//Spawn Missle
		if (sc) {
			FVector Direction = bHit ? (OutHit.Location - sc->GetComponentLocation()).GetSafeNormal() : (OutHit.TraceEnd - sc->GetComponentLocation()).GetSafeNormal();
			FRotator SpawnRotation = Direction.Rotation();

			UE_LOG(LogTemp, Error, TEXT("Shoot Location %s, Hitt? %s"), *SpawnRotation.ToString(), bHit ? *OutHit.GetActor()->GetName() : TEXT("No hit actor"));

			// Spawn the actor (replace AMyActor with your actor class)
			AShipProjectile* SpawnedActor = GetWorld()->SpawnActor<AShipProjectile>(weaponInfo.ProjectileClass, sc->GetComponentLocation(), SpawnRotation, SpawnParams);

		}

	}


}

void ASpaceShooter_3DCharacter::SwitchWeapon(const EWeaponType ArmorSlot)
{
	int MaxWeapon = ArmorSlot == EWeaponType::LightArmor ? m_ShipAttribute.LightArmor.Num() - 1 : m_ShipAttribute.HeavyArmor.Num() - 1;

	int CurrentWeapon = m_CurrentWeapon[ArmorSlot];

	if (MaxWeapon <= 0) {
		return;
	}

	if (CurrentWeapon < MaxWeapon) {
		CurrentWeapon++;
	}
	else {
		CurrentWeapon = 0;
	}

	m_CurrentWeapon.Add(ArmorSlot, CurrentWeapon);
	/*Reset the timer so it can work with correct shooting rate*/
	ResetShootingTimer(ArmorSlot);
}

void ASpaceShooter_3DCharacter::SetShip(const EShipType& ShipType)
{

	if (!m_DataTableAsset) {
		UE_LOG(LogTemp, Error, TEXT("Invalid DataTable"));
		return;
	}
	FString ShipString = UEnum::GetValueAsString(ShipType);
	const FShipAttribute* data = m_DataTableAsset->FindRow<FShipAttribute>(FName(ShipString), TEXT("Searching Ship data from data table"));

	if (!data) {
		UE_LOG(LogTemp, Error, TEXT("Invalid DataTable Row"));
		return;
	}

	m_ShipAttribute = *data;

	if (m_ShipAttribute.ShipMesh) {
		// Spawn the actor from the class
		ShipChildActor->SetChildActorClass(m_ShipAttribute.ShipMesh);
		UE_LOG(LogTemp, Error, TEXT("valid Ship Class"));
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Invalid Ship Class"));
	}
}




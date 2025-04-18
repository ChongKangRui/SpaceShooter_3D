// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipProjectile.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
AShipProjectile::AShipProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->SetCollisionProfileName("ProjectileCollision");
	RootComponent = CollisionComponent;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
}

void AShipProjectile::BeginPlay()
{
	Super::BeginPlay();
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AShipProjectile::OnProjectileOverlap);

}

void AShipProjectile::Tick(float Delta)
{
	Super::Tick(Delta);
	if (m_TrackTarget) {

	
		FVector NormalizeDir = (m_TrackTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		FRotator TargetRotation = NormalizeDir.Rotation();

		SetActorLocationAndRotation(
			FMath::Lerp(GetActorLocation(), m_TrackTarget->GetActorLocation(), FMath::Clamp(Delta * m_TrackingSpeed, 0.f, 1.0f)),
			FMath::Lerp(GetActorRotation(), TargetRotation, FMath::Clamp(Delta * m_TrackingSpeed,0.f, 1.0f)));
		
	}
	
}

void AShipProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetOwner() && OtherActor != GetInstigator()) {
		//UE_LOG(LogTemp, Error, TEXT("OtherActor: %s, Other Component: %s"), *OtherActor->GetName(), *OtherComp->GetName());
		//UE_LOG(LogTemp, Error, TEXT("Projectile Owner %s"), *GetOwner()->GetName());
		UGameplayStatics::ApplyDamage(OtherActor, m_Damage, GetInstigatorController(), GetInstigator(), nullptr);
		if (m_HitEffect)
		{
			// Define where to spawn the system
			FVector SpawnLocation = GetActorLocation() + (FMath::VRand() * HitRandomLocationMultiplier);
			FRotator SpawnRotation = GetActorRotation(); 

			// Spawn the Niagara system
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), m_HitEffect, SpawnLocation, SpawnRotation);
		}
		Destroy();
	}
	
}

void AShipProjectile::Initialization(float Damage, AActor* TrackTerget, float TrackRotateSpeed, UNiagaraSystem* HitEffect)
{
	m_Damage = Damage;
	m_TrackingSpeed = TrackRotateSpeed;
	m_TrackTarget = TrackTerget;
	m_HitEffect = HitEffect;
	//UE_LOG(LogTemp, Error, TEXT("Tracking Target? %s"), m_TrackingSpeed->GetName());


}



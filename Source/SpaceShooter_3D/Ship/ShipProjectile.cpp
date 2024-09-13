// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipProjectile.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AShipProjectile::AShipProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	RootComponent = CollisionComponent;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
	//ProjectileMovementComponent->InitialSpeed = 3000.0f;
	//ProjectileMovementComponent->MaxSpeed = 3000.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	//ProjectileMovementComponent->bShouldBounce = true;
	//ProjectileMovementComponent->Bounciness = 0.3f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
}

void AShipProjectile::HitMesh(AActor* hitActor)
{
	if (hitActor && hitActor != GetOwner()) {
		UGameplayStatics::ApplyDamage(hitActor, m_Damage, nullptr, GetOwner(), nullptr);
		Destroy();
	}
}

void AShipProjectile::Initialization(float Damage)
{
	m_Damage = Damage;
}



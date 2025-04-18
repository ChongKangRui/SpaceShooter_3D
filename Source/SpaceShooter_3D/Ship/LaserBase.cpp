// Fill out your copyright notice in the Description page of Project Settings.


#include "Ship/LaserBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"

ALaserBase::ALaserBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ProjectileMovementComponent->bInitialVelocityInLocalSpace = false;
	ProjectileMovementComponent->bSimulationEnabled = false;

	NiagaraEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraFX"));
	NiagaraEffect->SetupAttachment(RootComponent); // Attach to the root component

	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ALaserBase::InitializeLifeTime(float lifeTime)
{
	NiagaraEffect->SetFloatParameter(TEXT("User.RayLifeTime"), lifeTime);
	UE_LOG(LogTemp, Error, TEXT("LifeTime %f"), lifeTime);
	SetLaserLength(10000);
}

void ALaserBase::SetLaserLength(float length)
{
	//NiagaraEffect->SetRelativeScale3D(FVector(length,1.0,1.0));
	NiagaraEffect->SetFloatParameter(TEXT("User.Length"), length/ DivideScale);
	UE_LOG(LogTemp, Error, TEXT("LifeTime %f"), length);
	//NiagaraEffect->SetWorldScale3D(FVector(length,1.0, 1.0));
}

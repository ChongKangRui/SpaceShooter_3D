// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShipProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UENUM(Blueprintable)
enum EProjectileType {
	Missle,
	Laser
};

UCLASS()
class SPACESHOOTER_3D_API AShipProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShipProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;
protected:	
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TEnumAsByte<EProjectileType> ProjectileType;

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = Projectile)
	UProjectileMovementComponent* ProjectileMovementComponent;
};

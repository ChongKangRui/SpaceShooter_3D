// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShipProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class SPACESHOOTER_3D_API AShipProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShipProjectile();

	UFUNCTION(BlueprintCallable)
	void HitMesh(AActor* hitActor);

	void Initialization(float Damage);

protected:	
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = Projectile)
	UProjectileMovementComponent* ProjectileMovementComponent;

private:
	float m_Damage = 0.0f;
};

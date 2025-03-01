// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShipProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;

UCLASS()
class SPACESHOOTER_3D_API AShipProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShipProjectile();

	virtual void BeginPlay() override;
	virtual void Tick(float Delta) override;

	UFUNCTION()
	void OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void Initialization(float Damage, AActor* TrackTerget, float TrackRotateSpeed, UNiagaraSystem* HitEffect);

protected:	
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = Projectile)
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere, Category = Projectile)
	float HitRandomLocationMultiplier = 50.0f;
private:
	float m_Damage = 0.0f;
	float m_TrackingSpeed;
	AActor* m_TrackTarget;
	UNiagaraSystem* m_HitEffect;
};

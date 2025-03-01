// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShipProjectile.h"
#include "LaserBase.generated.h"


class UNiagaraComponent;

UCLASS()
class SPACESHOOTER_3D_API ALaserBase : public AShipProjectile
{
	GENERATED_BODY()

public:
	ALaserBase();
	void InitializeLifeTime(float lifeTime);
	void SetLaserLength(float length);

public:
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float DivideScale = 70;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	UNiagaraComponent* NiagaraEffect;
	
};

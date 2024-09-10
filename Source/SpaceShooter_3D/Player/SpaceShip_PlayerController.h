// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SpaceShip_PlayerController.generated.h"

/**
 * 
 */
class UInputMappingContext;
class UInputAction;
class UPlayerInputMapping;
class ASpaceShooter_Player;
struct FInputActionValue;

UCLASS()
class SPACESHOOTER_3D_API ASpaceShip_PlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	float GetCurrentPitchValue() const;
	float GetCurrentYawValue() const;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* p) override;

	void LoadPlayerMappingDataset();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TSoftObjectPtr<UPlayerInputMapping> PlayerMappingDataset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float SmoothNegateAxisValue = 0.1f;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StopLook();

	void StartSpeeding();
	void StopSpeeding();

	FVector LeftRightDirection() const;
	FVector ForwardBackDirection() const;

private:
	TObjectPtr<ASpaceShooter_Player> m_Player;

	FTimerHandle m_SmoothNegateRotation;

	float m_PitchValue = 0;
	float m_YawValue = 0;
};

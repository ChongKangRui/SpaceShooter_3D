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
struct FInputActionValue;

UCLASS()
class SPACESHOOTER_3D_API ASpaceShip_PlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* p) override;
	
	float GetCurrentPitchValue() const;
	float GetCurrentYawValue() const;
public:
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShootAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShootAutoTrackingMissleAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SpeedingAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
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
	class ASpaceShooter_Player* m_Player;

	FTimerHandle m_SmoothNegateRotation;

	float m_PitchValue = 0;
	float m_YawValue = 0;
};

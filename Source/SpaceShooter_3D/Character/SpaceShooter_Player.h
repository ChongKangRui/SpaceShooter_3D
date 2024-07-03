// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/SpaceShooter_3DCharacter.h"
#include "SpaceShooter_Player.generated.h"

/**
 * 
 */

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class SPACESHOOTER_3D_API ASpaceShooter_Player : public ASpaceShooter_3DCharacter
{
	GENERATED_BODY()
	
public:
	ASpaceShooter_Player();

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

public:

protected:
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;
	
	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float CameraFOV_InterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	FVector DesiredSocketOffset = FVector(0.0f, 0.0f, 50.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Flight, meta = (AllowPrivateAccess = "true"))
	float FlightInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Flight, meta = (AllowPrivateAccess = "true"))
	float SpeedingFOV = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Flight, meta = (AllowPrivateAccess = "true"))
	float NormalFOV = 90.0f;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	FRotator MaxAimRotation = FRotator(50.0f, 50.0f, 0.0f);

	
private:
	void UpdateRotationSetting();
	void UpdateCameraSetting();
	void UpdateRotationSmoothly(FRotator Target, float ConstantSpeed, float SmoothSpeed);

	FRotator CalculateFlightRotation() const;
private:
	FRotator CurrentFlightRotation;
	FRotator LastVelocityRotation;

	float TargetCameraFOV = 90.0f;

	friend class ASpaceShip_PlayerController;
};

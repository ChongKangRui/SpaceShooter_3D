// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SpaceShip_PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "Character/SpaceShooter_Player.h"
#include "GameFramework/CharacterMovementComponent.h"

void ASpaceShip_PlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
}

void ASpaceShip_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent)) {

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASpaceShip_PlayerController::Move);
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASpaceShip_PlayerController::Look);

		EnhancedInputComponent->BindAction(SpeedingAction, ETriggerEvent::Started , this, &ASpaceShip_PlayerController::StartSpeeding);
		EnhancedInputComponent->BindAction(SpeedingAction, ETriggerEvent::Completed, this, &ASpaceShip_PlayerController::StopSpeeding);
		EnhancedInputComponent->BindAction(SpeedingAction, ETriggerEvent::Canceled, this, &ASpaceShip_PlayerController::StopSpeeding);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASpaceShip_PlayerController::OnPossess(APawn* p)
{
	Super::OnPossess(p);
	if (p) {
		player = Cast<ASpaceShooter_Player>(p);

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(this->GetLocalPlayer()))
		{
			UE_LOG(LogTemp, Error, TEXT("Heeeeeee"));
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ASpaceShip_PlayerController::Move(const FInputActionValue& Value)
{
	if (!player)
		return;
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();


	const FVector RightDirection = LeftRightDirection();
	const FVector ForwardDirection = ForwardBackDirection();;

	// add movement 
	player->AddMovementInput(ForwardDirection, MovementVector.Y);
	player->AddMovementInput(RightDirection, MovementVector.X);

}

void ASpaceShip_PlayerController::Look(const FInputActionValue& Value)
{
	if (!player)
		return;

	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();


	// add yaw and pitch input to controller
	player->AddControllerYawInput(LookAxisVector.X);
	player->AddControllerPitchInput(LookAxisVector.Y);


}

void ASpaceShip_PlayerController::StartSpeeding()
{
	if (!player)
		return;

	player->TargetCameraFOV = player->SpeedingFOV;
	player->GetCharacterMovement()->MaxFlySpeed = 2000.0f;
}

void ASpaceShip_PlayerController::StopSpeeding()
{
	if (!player)
		return;

	player->TargetCameraFOV = player->NormalFOV;
	player->GetCharacterMovement()->MaxFlySpeed = 600.0f;
}

FVector ASpaceShip_PlayerController::LeftRightDirection() const
{
	// find out which way is forward
	const FRotator Rotation = GetControlRotation();
	//const FRotator YawRotation(0, Rotation.Yaw, Rotation.Roll);
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	return UKismetMathLibrary::GetRightVector(YawRotation);
}

FVector ASpaceShip_PlayerController::ForwardBackDirection() const
{
	const FRotator Rotation = GetControlRotation();

	bool ZInrange = UKismetMathLibrary::InRange_FloatFloat(Rotation.Vector().Z, -1, -0.1);
	bool ZInrange2 = UKismetMathLibrary::InRange_FloatFloat(Rotation.Vector().Z, 0.1, 1.0);

	float ZValue = ZInrange ? Rotation.Vector().Z : 0.0f + ZInrange2 ? Rotation.Vector().Z : 0.0f;

	const FVector OutVector = FVector(Rotation.Vector().X, Rotation.Vector().Y, ZValue);
	return OutVector;
}



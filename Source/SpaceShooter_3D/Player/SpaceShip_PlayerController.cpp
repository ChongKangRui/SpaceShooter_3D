// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceShip_PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "SpaceShooter_Player.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/PlayerInputMapping.h"

void ASpaceShip_PlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
}

void ASpaceShip_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!PlayerMappingDataset.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("PC: PlayerMappingData Reference Invalid"));
		return;
	}

	if (!m_Player) {
		UE_LOG(LogTemp, Error, TEXT("PC: Invalid Player Character Reference"));
		return;
	}

	// Set up action bindings
	if (UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent)) {
		if (const UInputAction* move = PlayerMappingDataset->MoveAction.Get()) {
			enhancedInputComponent->BindAction(move, ETriggerEvent::Triggered, this, &ASpaceShip_PlayerController::Move);
		}

		if (const UInputAction* look = PlayerMappingDataset->LookAction.Get()) {
			enhancedInputComponent->BindAction(look, ETriggerEvent::Triggered, this, &ASpaceShip_PlayerController::Look);
			enhancedInputComponent->BindAction(look, ETriggerEvent::Completed, this, &ASpaceShip_PlayerController::StopLook);
			enhancedInputComponent->BindAction(look, ETriggerEvent::Canceled, this, &ASpaceShip_PlayerController::StopLook);
		}

		if (const UInputAction* speeding = PlayerMappingDataset->SpeedingAction.Get()) {
			enhancedInputComponent->BindAction(speeding, ETriggerEvent::Started, this, &ASpaceShip_PlayerController::StartSpeeding);
			enhancedInputComponent->BindAction(speeding, ETriggerEvent::Completed, this, &ASpaceShip_PlayerController::StopSpeeding);
			enhancedInputComponent->BindAction(speeding, ETriggerEvent::Canceled, this, &ASpaceShip_PlayerController::StopSpeeding);
		}
		if (const UInputAction* LightArmorShoot = PlayerMappingDataset->LightArmorShootAction.Get()) {
			enhancedInputComponent->BindAction(LightArmorShoot, ETriggerEvent::Started, m_Player.Get(), &ASpaceShooter_3DCharacter::StartFireMissle, EWeaponType::LightArmor);
			enhancedInputComponent->BindAction(LightArmorShoot, ETriggerEvent::Completed, m_Player.Get(), &ASpaceShooter_3DCharacter::StopFireMissle, EWeaponType::LightArmor);
			enhancedInputComponent->BindAction(LightArmorShoot, ETriggerEvent::Canceled, m_Player.Get(), &ASpaceShooter_3DCharacter::StopFireMissle, EWeaponType::LightArmor);

		}

		
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	PlayerMappingDataset.Reset();
}

void ASpaceShip_PlayerController::OnPossess(APawn* p)
{
	Super::OnPossess(p);
	if (p) {
		m_Player = Cast<ASpaceShooter_Player>(p);

		/*Input Setup*/
		LoadPlayerMappingDataset();
		SetupInputComponent();
		SetInputMode(FInputModeGameOnly());

		/*Rotation Value Setup*/
		m_PitchValue = m_Player->GetActorRotation().Pitch;
		m_YawValue = m_Player->GetActorRotation().Yaw;
	}
}

void ASpaceShip_PlayerController::LoadPlayerMappingDataset()
{
	//Load the player mapping context and input action from the data asset
	if (PlayerMappingDataset.IsPending()) {
		PlayerMappingDataset.LoadSynchronous();
	}

	if (PlayerMappingDataset.IsValid()) {
		if (const UInputMappingContext* mappingContext = PlayerMappingDataset->DefaultMappingContext.Get()) {
			if (UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(this->GetLocalPlayer())) {
				subsystem->AddMappingContext(mappingContext, 0);
			}
		}
	}

}


float ASpaceShip_PlayerController::GetCurrentPitchValue() const
{
	return m_PitchValue;
}

float ASpaceShip_PlayerController::GetCurrentYawValue() const
{
	return m_YawValue;
}

void ASpaceShip_PlayerController::Move(const FInputActionValue& Value)
{
	if (!m_Player)
		return;
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();


	const FVector RightDirection = LeftRightDirection();
	const FVector ForwardDirection = ForwardBackDirection();;

	// add movement 
	m_Player->AddMovementInput(ForwardDirection, MovementVector.Y);
	m_Player->AddMovementInput(RightDirection, MovementVector.X);

}

void ASpaceShip_PlayerController::Look(const FInputActionValue& Value)
{
	if (!m_Player)
		return;

	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	m_YawValue = LookAxisVector.X;
	m_PitchValue = -LookAxisVector.Y;
}

void ASpaceShip_PlayerController::StopLook()
{
	m_YawValue = 0;
	m_PitchValue = 0;
}

void ASpaceShip_PlayerController::StartSpeeding()
{
	if (!m_Player)
		return;

	m_Player->m_TargetCameraFOV = m_Player->SpeedingFOV;
	m_Player->GetCharacterMovement()->MaxFlySpeed = 2000.0f;
}

void ASpaceShip_PlayerController::StopSpeeding()
{
	if (!m_Player)
		return;

	m_Player->m_TargetCameraFOV = m_Player->NormalFOV;
	m_Player->GetCharacterMovement()->MaxFlySpeed = 600.0f;
}

FVector ASpaceShip_PlayerController::LeftRightDirection() const
{
	// find out which way is forward
	const FRotator Rotation = m_Player->GetActorRotation();
	//const FRotator YawRotation(0, Rotation.Yaw, Rotation.Roll);
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	return UKismetMathLibrary::GetRightVector(YawRotation);
}

FVector ASpaceShip_PlayerController::ForwardBackDirection() const
{
	const FRotator Rotation = m_Player->GetActorRotation();

	bool ZInrange = UKismetMathLibrary::InRange_FloatFloat(Rotation.Vector().Z, -1, -0.1);
	bool ZInrange2 = UKismetMathLibrary::InRange_FloatFloat(Rotation.Vector().Z, 0.1, 1.0);

	float ZValue = ZInrange ? Rotation.Vector().Z : 0.0f + ZInrange2 ? Rotation.Vector().Z : 0.0f;

	const FVector OutVector = FVector(Rotation.Vector().X, Rotation.Vector().Y, ZValue);
	return OutVector;
}



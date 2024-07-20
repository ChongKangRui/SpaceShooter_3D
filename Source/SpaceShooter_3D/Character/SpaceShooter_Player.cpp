// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SpaceShooter_Player.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "SpaceShip_PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


ASpaceShooter_Player::ASpaceShooter_Player()
{
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	
	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void ASpaceShooter_Player::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	m_TargetCameraFOV = NormalFOV;

	m_PlayerController = Cast<ASpaceShip_PlayerController>(GetController());
	
}

void ASpaceShooter_Player::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	m_LastVelocityRotation = UKismetMathLibrary::MakeRotFromX(GetVelocity());

	UpdateCameraSetting();
	UpdateRotationSmoothly(CalculateFlightRotation(), 800.0f, FlightInterpSpeed);

}


void ASpaceShooter_Player::UpdateCameraSetting()
{
	float CamFOV = FMath::FInterpTo(GetFollowCamera()->FieldOfView, m_TargetCameraFOV, GetWorld()->GetDeltaSeconds(), CameraFOV_InterpSpeed);
	GetFollowCamera()->SetFieldOfView(CamFOV);

	FVector CameraBoomSocketOffset = FMath::VInterpTo(GetCameraBoom()->SocketOffset, DesiredSocketOffset, GetWorld()->GetDeltaSeconds(), 3.0f);
	GetCameraBoom()->SocketOffset = CameraBoomSocketOffset;
}

void ASpaceShooter_Player::UpdateRotationSmoothly(FRotator Target, float ConstantSpeed, float SmoothSpeed)
{
	//FQuat PitchQuat = FQuat(FRotator(m_PlayerController->GetCurrentPitchValue(), 0.0f, 0.0f));
	float PitchValue = m_PlayerController->GetCurrentPitchValue();
	float YawValue = m_PlayerController->GetCurrentYawValue();

	float PitchRotationTarget = GetShipTiltValue(YawValue, 0.5f, -FlightPitchTilt, FlightPitchTilt);
	float RollRotationTarget = GetShipTiltValue(PitchValue, 0.5f, FlightRollTilt, -FlightRollTilt);
	
	m_ShipMeshRotation.Pitch = FMath::FInterpTo(m_ShipMeshRotation.Pitch, PitchRotationTarget, GetWorld()->GetDeltaSeconds(), SmoothSpeed);
	m_ShipMeshRotation.Roll = FMath::FInterpTo(m_ShipMeshRotation.Roll, RollRotationTarget, GetWorld()->GetDeltaSeconds(), SmoothSpeed);
	
	ShipChildActor->SetRelativeRotation(FRotator(m_ShipMeshRotation.Pitch,0, m_ShipMeshRotation.Roll));

	UE_LOG(LogTemp, Error, TEXT("Pitch, %f , Yaw, %f"), PitchValue, YawValue);
	AddActorLocalRotation(FRotator(PitchValue, YawValue, 0.0f));
}
float ASpaceShooter_Player::GetShipTiltValue(float value, float Threshold, float NegativeTarget, float PositiveTarget)
{
	if (FMath::Abs(value) > Threshold) {
		if (value < 0.0f)
			return NegativeTarget;
		else
			return PositiveTarget;
	}
	return 0.0f;
}
FRotator ASpaceShooter_Player::CalculateFlightRotation() const
{
	//FRotator OutRotation;
	//float VelocityLength = FVector(GetVelocity().X, GetVelocity().Y, 0.0f).Length();

	return GetControlRotation();
}


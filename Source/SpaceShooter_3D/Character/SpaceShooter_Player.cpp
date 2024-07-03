// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SpaceShooter_Player.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
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
	TargetCameraFOV = NormalFOV;
	
}

void ASpaceShooter_Player::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	LastVelocityRotation = UKismetMathLibrary::MakeRotFromX(GetVelocity());

	UpdateCameraSetting();
	UpdateRotationSmoothly(CalculateFlightRotation(), 800.0f, FlightInterpSpeed);

}






void ASpaceShooter_Player::UpdateRotationSetting()
{
	FRotator AngleBetween = GetControlRotation() - GetActorRotation();
	AngleBetween.Normalize();
	UE_LOG(LogTemp, Error, TEXT("%s"), *AngleBetween.ToString());
	
	if(AngleBetween.Yaw > MaxAimRotation.Yaw || AngleBetween.Roll > MaxAimRotation.Roll)
		GetCharacterMovement()->bUseControllerDesiredRotation = true;


}

void ASpaceShooter_Player::UpdateCameraSetting()
{
	float CamFOV = FMath::FInterpTo(GetFollowCamera()->FieldOfView, TargetCameraFOV, GetWorld()->GetDeltaSeconds(), CameraFOV_InterpSpeed);
	GetFollowCamera()->SetFieldOfView(CamFOV);

	FVector CameraBoomSocketOffset = FMath::VInterpTo(GetCameraBoom()->SocketOffset, DesiredSocketOffset, GetWorld()->GetDeltaSeconds(), 3.0f);
	GetCameraBoom()->SocketOffset = CameraBoomSocketOffset;
}

void ASpaceShooter_Player::UpdateRotationSmoothly(FRotator Target, float ConstantSpeed, float SmoothSpeed)
{
	CurrentFlightRotation = FMath::RInterpConstantTo(CurrentFlightRotation, Target, GetWorld()->GetDeltaSeconds(), ConstantSpeed);
	
	FRotator FinalRotation = FMath::RInterpTo(GetActorRotation(), CurrentFlightRotation, GetWorld()->GetDeltaSeconds(), SmoothSpeed);
	SetActorRotation(FinalRotation);
}
FRotator ASpaceShooter_Player::CalculateFlightRotation() const
{
	FRotator OutRotation;
	float VelocityLength = FVector(GetVelocity().X, GetVelocity().Y, 0.0f).Length();

	return GetControlRotation();
}


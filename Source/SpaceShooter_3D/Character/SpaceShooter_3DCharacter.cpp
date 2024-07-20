// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpaceShooter_3DCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Pathfinding/AStarAgentComponent.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ASpaceShooter_3DCharacter

ASpaceShooter_3DCharacter::ASpaceShooter_3DCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(51.481384, 51.481384);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->MaxFlySpeed = 600.0;
	GetCharacterMovement()->BrakingDecelerationFlying = 150.0f;
	GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Flying;

	AgentComponent = CreateDefaultSubobject<UAStarAgentComponent>(TEXT("AStar Agent"));

	ShipChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("Ship"));
	ShipChildActor->SetupAttachment(GetMesh());
}






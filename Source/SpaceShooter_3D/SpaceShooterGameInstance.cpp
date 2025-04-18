// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceShooterGameInstance.h"

void USpaceShooterGameInstance::SetPlayerShipType(EShipType NewType)
{
	PlayerShipType = NewType;

}

EShipType USpaceShooterGameInstance::GetPlayerShipType() const
{
	return PlayerShipType;
}

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "SpaceShooter_3DCharacter.generated.h"


class UAStarAgentComponent;
class AShipProjectile;


DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UENUM(Blueprintable)
enum EShipType {
	GoldenVector,
	o_100,
	SpaceShip3
};

enum EWeaponType {
	LightArmor,
	HeavyArmor
};

USTRUCT(Blueprintable)
struct FShipArmor {
	GENERATED_BODY();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AShipProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Shoot_CD = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Shoot_Damage = 1.0f;
};


USTRUCT(Blueprintable)
struct FShipAttribute : public FTableRowBase{
	GENERATED_BODY();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ShipMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth;

	//Press 1 to switch armor
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FShipArmor> LightArmor;

	//Press 2 to switch armor
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FShipArmor> HeavyArmor;

};

UCLASS(config=Game)
class ASpaceShooter_3DCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASpaceShooter_3DCharacter();

	UFUNCTION(BlueprintCallable)
	void SetShip(const EShipType& ShipType);

	void StartFireMissle(const EWeaponType ArmorSlot);
	void StopFireMissle(const EWeaponType ArmorSlot);

	void SwitchWeapon(const EWeaponType ArmorSlot);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	bool bRandomizeMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true", EditCondition = "bRandomizeMesh = false"))
	TEnumAsByte<EShipType> ShipMeshType = EShipType::GoldenVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AIAgent, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAStarAgentComponent> AgentComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ChildACtor, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UChildActorComponent> ShipChildActor;


protected:
	virtual void BeginPlay() override;
	/*MissleSlot was refer to which missle to shoot*/

	void FireMissle(const EWeaponType ArmorSlot);

private:
	void DataTableAssetInitialization();

private:
	//Light Armor
	TArray<USceneComponent*> m_LightArmorFirePoint;
	//Heavy Armor
	TArray<USceneComponent*> m_HeavyArmorFirePoint;

	FTimerHandle m_LightArmor_Timer;
	FTimerHandle m_HeavyArmor_Timer;

	TArray<TSubclassOf<AActor>> m_ShipMeshArray;

	TMap<TEnumAsByte<EWeaponType>, int> m_CurrentWeapon;

	FShipAttribute m_ShipAttribute;
	TObjectPtr<UDataTable> m_DataTableAsset;
};


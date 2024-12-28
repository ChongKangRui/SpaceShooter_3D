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

	/*Reload or cool down after finish shooting*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Shoot_CD = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Shoot_Damage = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TraceRadius = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TraceDistance = 10000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ArmorAmount = 50;

	/*Only apply when it stop shooting and still have the armor*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ArmorRefillCD = 2.0f;

	/*Will cool down after armor was down to 0*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CoolDownCD = 4.0f;

	/*If not, will only damage when bullet hit the ship*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UseTraceApplyDamage = false;

	/*If laser, this will be different way of work, use Sphere trace apply damage will be activated immedially*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLaser;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bIsLaser"))
	float DamageApplyFrequency = 1.0f;

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
	FShipArmor LightArmor;

	//Press 2 to switch armor
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FShipArmor HeavyArmor;

};

UCLASS(config=Game)
class ASpaceShooter_3DCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASpaceShooter_3DCharacter();

	UFUNCTION(BlueprintCallable)
	void SetShip(const EShipType& ShipToUse);

	void StartFireMissle(const EWeaponType ArmorSlot);
	void StopFireMissle(const EWeaponType ArmorSlot);

	void SwitchWeapon(const EWeaponType ArmorSlot);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	bool bRandomizeMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true", EditCondition = "bRandomizeMesh = false"))
	TEnumAsByte<EShipType> ShipType = EShipType::GoldenVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AIAgent, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAStarAgentComponent> AgentComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ChildACtor, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UChildActorComponent> ShipChildActor;


protected:
	UFUNCTION()
	virtual void OnTakeAnyDamageBinding(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	virtual const FVector GetShootLocation() const;
	/*Normalized direction*/
	virtual const FVector GetShootDirection() const;
	virtual void BeginPlay() override;
	
	/*MissleSlot was refer to which missle to shoot*/

	void FireMissle(const EWeaponType ArmorSlot);
private:
	void DataTableAssetInitialization();
	
	void ResetShootingTimer(const EWeaponType ArmorSlot);

private:
	//Light Armor
	TArray<USceneComponent*> m_LightArmorFirePoint;
	//Heavy Armor
	TArray<USceneComponent*> m_HeavyArmorFirePoint;

	FTimerHandle m_LightArmor_Timer;
	FTimerHandle m_HeavyArmor_Timer;

	TArray<TSubclassOf<AActor>> m_ShipMeshArray;

	TMap<TEnumAsByte<EWeaponType>, int> m_CurrentWeaponArmorAmount;

	FShipAttribute m_ShipAttribute;
	TObjectPtr<UDataTable> m_DataTableAsset;

	float m_CurrentHealth;

};


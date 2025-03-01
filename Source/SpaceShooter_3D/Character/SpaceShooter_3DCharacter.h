// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "SpaceShooter_3DCharacter.generated.h"


class UAStarAgentComponent;
class AShipProjectile;
class ALaserBase;
class UNiagaraSystem;
class UNiagaraComponent;
class UPaperSprite;


DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UENUM(Blueprintable)
enum EShipType {
	GoldenVector,
	o_100,
	SpaceShip3
};

UENUM(Blueprintable)
enum EWeaponType {
	Light,
	Heavy
};

USTRUCT(Blueprintable)
struct FShipWeapon {
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
	int AmmunitionAmount = 50;

	/*Only apply when it stop shooting and still have the armor*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ArmorRefillCD = 2.0f;

	/*Will cool down after armor was down to 0*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CoolDownCD = 4.0f;

	/*If not, will only damage when bullet hit the ship*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool UseTraceApplyDamage = false;

	/*If laser, this will be different way of work, use Sphere trace apply damage will be activated immedially, damage frequency will be using Shoot_CD*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLaser;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MissleTracking)
	bool CanTrackEnemy = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MissleTracking)
	float TrackingRotateSpeed = 10.0f;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bIsLaser"))
	//float DamageApplyFrequency = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VFX)
	UNiagaraSystem* HitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPaperSprite* WeaponIcon;
};


USTRUCT(Blueprintable)
struct FShipAttribute : public FTableRowBase{
	GENERATED_BODY();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ShipMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.0f;

	//Press 1 to switch armor
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FShipWeapon LightWeapon;

	//Press 2 to switch armor
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FShipWeapon HeavyWeapon;

};

UCLASS(config=Game)
class ASpaceShooter_3DCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASpaceShooter_3DCharacter();

	UFUNCTION(BlueprintCallable)
	void SetShip(const EShipType& ShipToUse);

	UFUNCTION(BlueprintCallable)
	bool IsInitialized() const;

	UFUNCTION(BlueprintPure)
	bool GetIsWeaponLaser(EWeaponType WeaponSlot) const;

	UFUNCTION(BlueprintPure)
	UPaperSprite* GetWeaponIcon(EWeaponType WeaponSlot) const;

	UFUNCTION(BlueprintPure)
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure)
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintPure)
	float GetCurrentWeaponAmmunition(TEnumAsByte<EWeaponType> weaponType) const;

	UFUNCTION(BlueprintPure)
	float GetMaxWeaponAmmunition(TEnumAsByte<EWeaponType> weaponType) const;




	void FireWeapon(const EWeaponType WeaponSlot);

	//void StartFireMissle(const EWeaponType WeaponSlot);
	
	virtual void StopWeapon(const EWeaponType WeaponSlot);
	
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
	virtual const FVector GetRandomShootOffset() const;
	/*This will mainly just used for AI*/
	virtual AActor* GetMissleTrackEnemy() const;
	virtual void BeginPlay() override;
	
	/*MissleSlot was refer to which missle to shoot*/

	void FireMissle(const EWeaponType WeaponSlot);
	void FireLaser(const EWeaponType WeaponSlot);

	bool CanShoot(const EWeaponType WeaponSlot) const;
	bool IsAmmoOut(const EWeaponType WeaponSlot) const;
	void StartWeaponTimer(const EWeaponType WeaponSlot);

private:
	//void DataTableAssetInitialization();

private:
	//Light Armor
	TArray<USceneComponent*> m_LightWeaponFirePoint;
	//Heavy Armor
	TArray<USceneComponent*> m_HeavyWeaponFirePoint;
	TArray<ALaserBase*> m_LaserInstance;

	FTimerHandle m_LightWeaponReload_Timer;
	FTimerHandle m_HeavyWeaponReload_Timer;

	TArray<TSubclassOf<AActor>> m_ShipMeshArray;

	TMap<TEnumAsByte<EWeaponType>, int> m_CurrentWeaponAmmunitionAmount;
	TMap<TEnumAsByte<EWeaponType>, bool> m_AllowToFireMissle;
	

	FShipAttribute m_ShipAttribute;
	//TObjectPtr<UDataTable> m_DataTableAsset;
	TWeakObjectPtr<UNiagaraComponent> m_LaserVFXInstance;


	float m_CurrentHealth;

};


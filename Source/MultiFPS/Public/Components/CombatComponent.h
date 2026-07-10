// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class UWeaponData;
class AFPSWeapon;
struct FGameplayTag;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MULTIFPS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void InitiateCycleWeapon();
	void InitiateFireWeapon_Pressed();
	void InitiateFireWeapon_Released();
	void InitiateReloadWeapon();
	void InitiateAim_Pressed();
	void InitiateAim_Released();
	
	void EquipWeapon(AFPSWeapon* Weapon);
	void SpawnInventory();
	void DestroyInventory();
	
	UPROPERTY(EditDefaultsOnly, Category="MFPS|Weapon")
	TObjectPtr<UWeaponData> WeaponData;	
	
protected:
	
	
private:
	
	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
	TObjectPtr<AFPSWeapon> CurrentWeapon;
	
	UFUNCTION()
	void OnRep_CurrentWeapon(const AFPSWeapon* LastWeapon) const;
	
	UPROPERTY(Transient, Replicated)
	TArray<AFPSWeapon*> Inventory;
	
	UPROPERTY(EditDefaultsOnly, Category="MFPS|Weapon")
	TArray<TSubclassOf<AFPSWeapon>> DefaultWeaponClasses;
	
	AFPSWeapon* SpawnWeapon(TSubclassOf<AFPSWeapon> WeaponClass) const;
};

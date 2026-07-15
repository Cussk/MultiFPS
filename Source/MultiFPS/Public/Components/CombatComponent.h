// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

struct FHitResult;
class UWeaponData;
class AFPSWeapon;
struct FGameplayTag;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAiming, bool, isAiming);

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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MFPS|Weapon")
	TObjectPtr<UWeaponData> WeaponData;	
	
	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentWeapon)
	TObjectPtr<AFPSWeapon> CurrentWeapon;
	
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bAiming;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "MFPS|Weapon")
	float TraceLength;
	
private:
	UPROPERTY(Transient, Replicated)
	TArray<AFPSWeapon*> Inventory;
	
	UPROPERTY(EditDefaultsOnly, Category="MFPS|Weapon")
	TArray<TSubclassOf<AFPSWeapon>> DefaultWeaponClasses;
	
	UPROPERTY(BlueprintAssignable)
	FOnAiming OnAiming;
	
	bool bTriggerPressed = false;
	FTimerHandle FireTimerHandle;
	
	UFUNCTION()
	void OnRep_CurrentWeapon(AFPSWeapon* LastWeapon) const;
	
	UFUNCTION(Server, Reliable)
	void Server_Aim(bool bPressed);
	
	UFUNCTION(Server, Reliable)
	void Server_FireWeapon(const FHitResult& Hit);
	
	UFUNCTION(Server, Reliable)
	void Server_DryFireWeapon();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FireWeapon(const FHitResult& Hit, int32 AuthAmmo);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DryFireWeapon();
	
	void Local_Aim(bool bPressed);
	void Local_FireWeapon();
	void Local_DryFireWeapon();
	
	AFPSWeapon* SpawnWeapon(TSubclassOf<AFPSWeapon> WeaponClass) const;
	void HandleCurrentWeaponChanged(AFPSWeapon* LastWeapon) const;
	void FireTimerFinished();
	
	
};

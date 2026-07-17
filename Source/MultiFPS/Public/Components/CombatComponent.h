// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "CombatComponent.generated.h"

class UAnimMontage;
class UMaterialInstanceDynamic;
struct FHitResult;
class UWeaponData;
class AMFPSWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAiming, bool, bisAiming);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTargetingPlayerStatusChanged, bool, bTargetPlayerChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCurrentReserveAmmoChanged, int32, RoundsInReserve, int32, RoundsInWeapon, UMaterialInterface*, WeaponIconMaterial);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRoundFired, int32, RoundsCurrent, int32, RoundsMax, int32, RoundsInReserve);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FReticleChanged, UMaterialInstanceDynamic*, ReticleMaterialInstanceDynamic, const FReticleParams&, ReticleParams, bool, bCurrentlyTargetingPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAmmoCounterChanged, UMaterialInstanceDynamic*, AmmoCounterMaterialInstanceDynamic, int32, RoundsCurrent, int32, RoundsMax);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MULTIFPS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintPure, Category = "MFPS|Combat")
	static UCombatComponent* GetCombatComponent(const AActor* Actor) { return IsValid(Actor) ? Actor->FindComponentByClass<UCombatComponent>() : nullptr; }

	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(AMFPSWeapon* Weapon);
	
	void InitiateCycleWeapon();
	void InitiateFireWeapon_Pressed();
	void InitiateFireWeapon_Released();
	void InitiateReloadWeapon();
	void InitiateAim_Pressed();
	void InitiateAim_Released();
	
	void Notify_CycleWeapon();
	
	void EquipWeapon(AMFPSWeapon* Weapon);
	void SpawnInventory();
	void DestroyInventory();
	
	void InitializeWeaponWidgets() const;
	
	UPROPERTY(BlueprintAssignable)
	FOnAiming OnAiming;
	
	UPROPERTY(BlueprintAssignable)
	FTargetingPlayerStatusChanged OnTargetingPlayerStatusChanged;
	
	UPROPERTY(BlueprintAssignable)
	FCurrentReserveAmmoChanged OnCurrentReserveAmmoChanged;
	
	UPROPERTY(BlueprintAssignable)
	FReticleChanged OnReticleChanged;
	
	UPROPERTY(BlueprintAssignable)
	FAmmoCounterChanged OnAmmoCounterChanged;
	
	UPROPERTY(BlueprintAssignable)
	FRoundFired OnRoundFired;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MFPS|Weapon")
	TObjectPtr<UWeaponData> WeaponData;	
	
	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentWeapon)
	TObjectPtr<AMFPSWeapon> CurrentWeapon;
	
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bAiming;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentReserveAmmo)
	int32 CurrentReserveAmmo;
	
	bool bHitPlayer = false;
	
protected:
	UFUNCTION()
	void BlendOut_CycleWeapon(UAnimMontage* Montage, bool bInterrupted);
	
	UPROPERTY(EditDefaultsOnly, Category = "MFPS|Weapon")
	float TraceLength;
	
private:	
	UFUNCTION(Server, Reliable)
	void Server_Aim(bool bPressed);
	
	UFUNCTION(Server, Reliable)
	void Server_FireWeapon(const FHitResult& Hit);
	
	UFUNCTION(Server, Reliable)
	void Server_DryFireWeapon();
	
	UFUNCTION(Server, Reliable)
	void Server_CycleWeapon(int32 WeaponIndex);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FireWeapon(const FHitResult& Hit, int32 AuthAmmo);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DryFireWeapon();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CycleWeapon(int32 WeaponIndex);
	
	void Local_Aim(bool bPressed);
	void Local_FireWeapon();
	void Local_DryFireWeapon();
	void Local_CycleWeapon(int32 WeaponIndex);
	
	int32 AdvanceWeaponIndex();
	
	AMFPSWeapon* SpawnWeapon(TSubclassOf<AMFPSWeapon> WeaponClass) const;
	void SetCurrentWeapon(AMFPSWeapon* NewWeapon, AMFPSWeapon* LastWeapon);
	void FireTimerFinished();
	
	UFUNCTION()
	void OnRep_CurrentReserveAmmo();
	
	UFUNCTION()
	void OnRep_CurrentWeapon(AMFPSWeapon* LastWeapon);
	
	UPROPERTY(Transient, Replicated)
	TArray<AMFPSWeapon*> Inventory;
	
	UPROPERTY(EditDefaultsOnly, Category="MFPS|Weapon")
	TArray<TSubclassOf<AMFPSWeapon>> DefaultWeaponClasses;
	
	TMap<FGameplayTag, int32> ReserveAmmo;
	bool bTriggerPressed = false;
	FTimerHandle FireTimerHandle;
	bool bHitPlayerLastFrame = false;
	
	int32 Local_WeaponIndex;
};

// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Types/MFPSTypes.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"

class AMFPSWeapon;

UINTERFACE()
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MULTIFPS_API IPlayerInterface
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FWeaponSocketAlignment GetTPWeaponSocketAlignment(const FGameplayTag& WeaponType) const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FWeaponSocketAlignment GetFPWeaponSocketAlignment(const FGameplayTag& WeaponType) const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetMeshFirstPerson() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetMeshThirdPerson() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	AMFPSWeapon* GetCurrentWeapon() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetCurrentWeaponReserveAmmo() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void WeaponReplicated();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Notify_CycleWeapon();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Notify_ReloadWeapon();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddAmmo(const FGameplayTag WeaponType, int32 AmmoAmount);
};

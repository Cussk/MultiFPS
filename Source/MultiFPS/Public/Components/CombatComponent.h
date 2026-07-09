// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/WeaponTypes.h"
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
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void InitiateCycleWeapon();
	void InitiateFireWeapon_Pressed();
	void InitiateFireWeapon_Released();
	void InitiateReloadWeapon();
	void InitiateAim_Pressed();
	void InitiateAim_Released();
	
	void SpawnInventory();
	void DestroyInventory();
	
	UPROPERTY(EditDefaultsOnly, Category="MFPS|Weapon")
	TObjectPtr<UWeaponData> WeaponData;	
	
protected:
	
	
private:
	
	UPROPERTY(EditDefaultsOnly, Category="MFPS|Weapon")
	TSubclassOf<AFPSWeapon> DefaultWeaponClass;
	
	AFPSWeapon* SpawnWeapon(TSubclassOf<AFPSWeapon> WeaponClass) const;
};

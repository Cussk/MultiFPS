// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "Types/WeaponTypes.h"
#include "WeaponData.generated.h"

/**
 * 
 */
UCLASS()
class MULTIFPS_API UWeaponData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "MFPS|WeaponData|Weapons")
	TMap<FGameplayTag, FWeaponSocketAlignment> TPWeaponSocketAlignments;
	
	UPROPERTY(EditDefaultsOnly, Category = "MFPS|WeaponData|Weapons")
	TMap<FGameplayTag, FWeaponSocketAlignment> FPWeaponSocketAlignments;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MFPS|WeaponData|FirstPerson")
	TMap<FGameplayTag, FPlayerAnims> FirstPersonAnims;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MFPS|WeaponData|ThirdPerson")
	TMap<FGameplayTag, FPlayerAnims> ThirdPersonAnims;
};

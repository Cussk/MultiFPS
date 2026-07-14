// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "Types/MFPSTypes.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"

struct FGameplayTag;

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
};

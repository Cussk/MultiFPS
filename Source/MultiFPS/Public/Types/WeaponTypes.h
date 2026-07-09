#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.generated.h"

USTRUCT(BlueprintType)
struct FWeaponSocketAlignment
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	FVector SocketLocation;
	
	UPROPERTY(EditDefaultsOnly)
	FRotator SocketRotation;
	
	UPROPERTY(EditDefaultsOnly)
	FVector SocketScale = FVector(1.0f, 1.0f, 1.0f);
};


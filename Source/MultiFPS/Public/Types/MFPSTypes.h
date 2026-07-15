#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimSequence.h"
#include "MFPSTypes.generated.h"

UENUM(BlueprintType)
enum class ETurnInPlace : uint8
{
	Left UMETA(DisplayName = "TurningLeft"),
	Right UMETA(DisplayName = "TurningRight"),
	NotTurning UMETA(DisplayName = "NotTurning")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	Auto UMETA(DisplayName = "Automatic"),
	SemiAuto UMETA(DisplayName = "SemiAutomatic")
};

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

USTRUCT(BlueprintType)
struct FPlayerAnims
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimSequence> IdleAnim = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimSequence> AimIdleAnim = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimSequence> CrouchIdleAnim = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimSequence> SprintAnim = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UBlendSpace> AimOffset_Hip;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UBlendSpace> AimOffset_Aim;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UBlendSpace> Strafe_Standing;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UBlendSpace> Strafe_Crouching;
};

USTRUCT(BlueprintType)
struct FMontageData
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> EquipMontage = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> ReloadMontage = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> FireMontage = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> DryFireMontage = nullptr;
};


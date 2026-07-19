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

UENUM(BlueprintType)
enum class EWeaponStatus : uint8
{
	Idle,
	Firing,
	DryFiring,
	Reloading,
	Cycling,
	Unequipped
};

UENUM(BlueprintType)
enum class EDeathState : uint8
{
	NotDead,
	DeathStarted,
	DeathFinished
};

UENUM(meta = (BitFlags))
enum class ESpecialKillTypes : uint16
{
	None = 0,					// 00000000 00000000
	Headshot = 1 << 0,			// 00000000 00000001
	Sequential = 1 << 1,		// 00000000 00000010
	Streak = 1 << 2,			// 00000000 00000100
	Revenge = 1 << 3,			// 00000000 00001000
	Dethrone = 1 << 4,			// 00000000 00010000
	Showstopper = 1 << 5,		// 00000000 00100000
	FirstBlood = 1 << 6,		// 00000000 01000000
	GainedTheLead = 1 << 7,		// 00000000 10000000
	TiedTheLeader = 1 << 8,		// 00000001 00000000
	LostTheLead = 1 << 9,		// 00000010 00000000
};

ENUM_CLASS_FLAGS(ESpecialKillTypes)

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

USTRUCT(BlueprintType)
struct FReticleParams
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ShapeCutFactor_RoundFired = 0.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ShapeCutFactor_Default = 0.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ShapeCutFactor_Aiming = 0.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_RoundFired = 0.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_Default = -0.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_Aiming = -0.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_Targeting = -0.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_NotTargeting = -0.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RoundFiredInterpSpeed = 20.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AimingInterpSpeed = 15.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TargetingInterpSpeed = 10.0f;
};

USTRUCT(BlueprintType)
struct FFiredRoundReport
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	AActor* Attacker = nullptr; 
	
	UPROPERTY(BlueprintReadOnly)
	AActor* Victim = nullptr; 
	
	UPROPERTY(BlueprintReadOnly)
	bool bHit = false;
	
	UPROPERTY(BlueprintReadOnly)
	bool bHeadShot = false; 
	
	UPROPERTY(BlueprintReadOnly)
	bool bLethal = false;
};

USTRUCT(BlueprintType)
struct FSpecialElimInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	ESpecialKillTypes SpecialKillType = ESpecialKillTypes::None;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString ElimMessage = FString();
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TObjectPtr<UTexture2D> ElimIcon = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	int32 SequentialKillCount = 0;
	
	UPROPERTY(BlueprintReadOnly)
	int32 StreakCount = 0;
};

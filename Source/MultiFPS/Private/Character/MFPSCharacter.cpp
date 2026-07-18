// Copyright Kyle Cuss and Cuss Programming


#include "Character/MFPSCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/WeaponData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/MFPSWeapon.h"

namespace MFPSCharacterConstants
{
	FName FABRIK_OriginSocketName(TEXT("hand_r"));
	FName FABRIK_SocketName(TEXT("FABRIK_Socket"));
}

AMFPSCharacter::AMFPSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	GetCharacterMovement()->MovementState.bCanCrouch = true;
	
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 0.0f;
	SpringArmComponent->bEnableCameraLag = true;
	SpringArmComponent->CameraLagSpeed = 15.0f;
	SpringArmComponent->bUsePawnControlRotation = true;
	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>("FirstPersonCamera");
	FirstPersonCameraComponent->SetupAttachment(SpringArmComponent);
	FirstPersonCameraComponent->bUsePawnControlRotation = false;
	
	MeshFirstPerson = CreateDefaultSubobject<USkeletalMeshComponent>("MeshFirstPerson");
	MeshFirstPerson->SetupAttachment(FirstPersonCameraComponent);
	MeshFirstPerson->bOnlyOwnerSee = true;
	MeshFirstPerson->bOwnerNoSee = false;
	MeshFirstPerson->bCastDynamicShadow = false;
	MeshFirstPerson->bReceivesDecals = false;
	MeshFirstPerson->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	MeshFirstPerson->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	
	GetMesh()->bOnlyOwnerSee = false;
	GetMesh()->bOwnerNoSee = true;
	GetMesh()->bReceivesDecals = false;
	
	CombatComponent = CreateDefaultSubobject<UCombatComponent>("CombatComponent");
	CombatComponent->SetIsReplicated(true);
	
	DefaultFOV = 90.0f;
	TurningStatus = ETurnInPlace::NotTurning;
	bWeaponFirstReplicated = false;
}

void AMFPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	FirstPersonCameraComponent->SetFieldOfView(DefaultFOV);
	
	StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
}

void AMFPSCharacter::BeginDestroy()
{
	if (IsValid(CombatComponent))
	{
		CombatComponent->DestroyInventory();
	}
	
	Super::BeginDestroy();
}

void AMFPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	CalculateTurnInPlaceParameters(DeltaTime);
	CalculateFABRIKSocketTransforms();
}

void AMFPSCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	CombatComponent->SpawnInventory();
}

void AMFPSCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	if (IsValid(CombatComponent))
	{
		CombatComponent->InitializeWeaponWidgets();
	}
}

UCombatComponent* AMFPSCharacter::GetCombatComponent()
{
	return CombatComponent;
}

bool AMFPSCharacter::HasCurrentWeapon()
{
	return IsValid(CombatComponent) && CombatComponent->CurrentWeapon != nullptr;
}

// Fix compressed rotation data from GetGaseAimRotation
FRotator AMFPSCharacter::GetFixedAimRotation() const
{
	FRotator AimRotation = GetBaseAimRotation();
	if (AimRotation.Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0]
		const FVector2D InRange(270.f, 360.f);
		const FVector2D OutRange(-90.f, 0.f);
		AimRotation.Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AimRotation.Pitch);
	}
	
	return AimRotation;
}

void AMFPSCharacter::CalculateFABRIKSocketTransforms()
{
	if (IsValid(CombatComponent) && IsValid(CombatComponent->CurrentWeapon) && IsValid(CombatComponent->CurrentWeapon->GetMeshThirdPerson()))
	{
		FABRIK_SocketTransform = CombatComponent->CurrentWeapon->GetMeshThirdPerson()->GetSocketTransform(MFPSCharacterConstants::FABRIK_SocketName, RTS_World);
		
		FVector OutLocation;
		FRotator OutRotation;
		GetMesh()->TransformToBoneSpace(MFPSCharacterConstants::FABRIK_OriginSocketName, FABRIK_SocketTransform.GetLocation(), 
			FABRIK_SocketTransform.GetRotation().Rotator(), OutLocation, OutRotation);
		
		FABRIK_SocketTransform.SetLocation(OutLocation);
		FABRIK_SocketTransform.SetRotation(OutRotation.Quaternion());
	}
}

void AMFPSCharacter::CalculateTurnInPlaceParameters(const float DeltaTime)
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	const float Speed = Velocity.Size();
	const bool bIsInAir = GetCharacterMovement()->IsFalling();
	
	if (Speed == 0.0f && !bIsInAir)
	{
		const FRotator CurrentAimRotation(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AOYaw = DeltaAimRotation.Yaw;
		
		if (TurningStatus == ETurnInPlace::NotTurning)
		{
			InterpAOYaw = AOYaw;
		}
		
		TurnInPlace(DeltaTime);
	}
	
	if (Speed > 0.0f || bIsInAir)
	{
		StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		AOYaw = 0.0f;
		
		FRotator AimRotation = GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(GetVelocity());
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		
		TurningStatus = ETurnInPlace::NotTurning;
	}
	
	AOYaw *= -1.0f;
}

void AMFPSCharacter::TurnInPlace(const float DeltaTime)
{
	if (AOYaw > 90.0f)
	{
		TurningStatus = ETurnInPlace::Right;
	}
	else if (AOYaw < -90.0f)
	{
		TurningStatus = ETurnInPlace::Left;
	}
	
	if (TurningStatus != ETurnInPlace::NotTurning)
	{
		InterpAOYaw = FMath::FInterpTo(InterpAOYaw, 0.0f, DeltaTime, TurnSpeed);
		AOYaw = InterpAOYaw;
		if (FMath::Abs(AOYaw) < 5.0f)
		{
			TurningStatus = ETurnInPlace::NotTurning;
			StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		}
	}
}


FWeaponSocketAlignment AMFPSCharacter::GetTPWeaponSocketAlignment_Implementation(const FGameplayTag& WeaponType) const
{
	checkf(CombatComponent->WeaponData, TEXT("No Weapon Data set on CombatComponent"))
	return CombatComponent->WeaponData->TPWeaponSocketAlignments.FindChecked(WeaponType);
}

FWeaponSocketAlignment AMFPSCharacter::GetFPWeaponSocketAlignment_Implementation(const FGameplayTag& WeaponType) const
{
	checkf(CombatComponent->WeaponData, TEXT("No Weapon Data set on CombatComponent"))
	return CombatComponent->WeaponData->FPWeaponSocketAlignments.FindChecked(WeaponType);
}

USkeletalMeshComponent* AMFPSCharacter::GetMeshFirstPerson_Implementation() const
{
	return MeshFirstPerson;
}

USkeletalMeshComponent* AMFPSCharacter::GetMeshThirdPerson_Implementation() const
{
	return GetMesh();
}

AMFPSWeapon* AMFPSCharacter::GetCurrentWeapon_Implementation() const
{
	return CombatComponent->CurrentWeapon;
}

int32 AMFPSCharacter::GetCurrentWeaponReserveAmmo_Implementation() const
{
	return CombatComponent->CurrentReserveAmmo;
}

void AMFPSCharacter::WeaponReplicated_Implementation()
{
	if (bWeaponFirstReplicated)
	{
		bWeaponFirstReplicated = true;
		OnWeaponFirstReplicated.Broadcast(CombatComponent->CurrentWeapon, CombatComponent->bHitPlayer);
	}
}

void AMFPSCharacter::Notify_CycleWeapon_Implementation()
{
	CombatComponent->Notify_CycleWeapon();
}

void AMFPSCharacter::Notify_ReloadWeapon_Implementation()
{
	CombatComponent->Notify_ReloadWeapon();
}

void AMFPSCharacter::AddAmmo_Implementation(const FGameplayTag WeaponType, int32 AmmoAmount)
{
	if (HasAuthority())
	{
		CombatComponent->AddAmmo(WeaponType, AmmoAmount);
	}
}

// Copyright Kyle Cuss and Cuss Programming


#include "Character/MFPSCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/WeaponData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

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
}

void AMFPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	FirstPersonCameraComponent->SetFieldOfView(DefaultFOV);
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
}

void AMFPSCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	CombatComponent->SpawnInventory();
}

UCombatComponent* AMFPSCharacter::GetCombatComponent()
{
	return CombatComponent;
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

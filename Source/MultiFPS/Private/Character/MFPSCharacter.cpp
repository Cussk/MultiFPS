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
}

void AMFPSCharacter::BeginPlay()
{
	Super::BeginPlay();
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

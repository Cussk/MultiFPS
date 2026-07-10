// Copyright Kyle Cuss and Cuss Programming


#include "Weapon/FPSWeapon.h"

#include "Character/MFPSCharacter.h"
#include "Components/SkeletalMeshComponent.h"

namespace WeaponConstants
{
	const FName WeaponSocketName(TEXT("WeaponGrip"));
}

AFPSWeapon::AFPSWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
	
	MeshFirstPerson = CreateDefaultSubobject<USkeletalMeshComponent>("MeshFirstPerson");
	MeshFirstPerson->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	MeshFirstPerson->bReceivesDecals = false;
	MeshFirstPerson->CastShadow = false;
	SetRootComponent(MeshFirstPerson);
	
	MeshThirdPerson = CreateDefaultSubobject<USkeletalMeshComponent>("MeshThirdPerson");
	MeshThirdPerson->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	MeshThirdPerson->bReceivesDecals = false;
	MeshThirdPerson->CastShadow = true;
	MeshThirdPerson->SetupAttachment(MeshFirstPerson);
	
	HideMeshes();
}

void AFPSWeapon::OnRep_Instigator()
{
	Super::OnRep_Instigator();
	AttachToOwningPawn();
}

void AFPSWeapon::BeginPlay()
{
	Super::BeginPlay();
}

USkeletalMeshComponent* AFPSWeapon::GetMeshFirstPerson() const
{
	return MeshFirstPerson;
}

USkeletalMeshComponent* AFPSWeapon::GetMeshThirdPerson() const
{
	return MeshThirdPerson;
}

void AFPSWeapon::AttachToOwningPawn() const
{
	APawn* OwningPawn = GetInstigator();
	if (!IsValid(OwningPawn) || !OwningPawn->Implements<UPlayerInterface>())
	{
		return;
	}
	
	SetMeshVisibilities(OwningPawn);
	
	const FWeaponSocketAlignment TPSocketAlignment = IPlayerInterface::Execute_GetTPWeaponSocketAlignment(OwningPawn, WeaponTypeTag);
	const FWeaponSocketAlignment FPSocketAlignment = IPlayerInterface::Execute_GetFPWeaponSocketAlignment(OwningPawn, WeaponTypeTag);
	USkeletalMeshComponent* PawnMeshFirstPerson = IPlayerInterface::Execute_GetMeshFirstPerson(OwningPawn);
	USkeletalMeshComponent* PawnMeshThirdPerson = IPlayerInterface::Execute_GetMeshThirdPerson(OwningPawn);
	
	MeshFirstPerson->AttachToComponent(PawnMeshFirstPerson, FAttachmentTransformRules::KeepRelativeTransform, WeaponConstants::WeaponSocketName);
	MeshFirstPerson->SetRelativeLocation(FPSocketAlignment.SocketLocation);
	MeshFirstPerson->SetRelativeRotation(FPSocketAlignment.SocketRotation);
	MeshFirstPerson->SetRelativeScale3D(FPSocketAlignment.SocketScale);
	
	MeshThirdPerson->AttachToComponent(PawnMeshThirdPerson, FAttachmentTransformRules::KeepRelativeTransform, WeaponConstants::WeaponSocketName);
	MeshThirdPerson->SetRelativeLocation(TPSocketAlignment.SocketLocation);
	MeshThirdPerson->SetRelativeRotation(TPSocketAlignment.SocketRotation);
	MeshThirdPerson->SetRelativeScale3D(TPSocketAlignment.SocketScale);
}

void AFPSWeapon::SetMeshVisibilities(const APawn* OwningPawn) const
{
	if (OwningPawn->IsLocallyControlled())
	{
		MeshFirstPerson->SetHiddenInGame(false);
		MeshThirdPerson->SetHiddenInGame(true);
	}
	else
	{
		MeshFirstPerson->SetHiddenInGame(true);
		MeshThirdPerson->SetHiddenInGame(false);
	}
}

void AFPSWeapon::HideMeshes() const
{
	MeshFirstPerson->SetHiddenInGame(true);
	MeshThirdPerson->SetHiddenInGame(true);
}

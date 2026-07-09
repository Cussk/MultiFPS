// Copyright Kyle Cuss and Cuss Programming


#include "Weapon/FPSWeapon.h"

#include "Character/MFPSCharacter.h"
#include "Components/SkeletalMeshComponent.h"

AFPSWeapon::AFPSWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
	
	MeshFirstPerson = CreateDefaultSubobject<USkeletalMeshComponent>("MeshFirstPerson");
	MeshFirstPerson->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	MeshFirstPerson->bReceivesDecals = false;
	MeshFirstPerson->CastShadow = false;
	//MeshFirstPerson->SetHiddenInGame(true);
	SetRootComponent(MeshFirstPerson);
	
	MeshThirdPerson = CreateDefaultSubobject<USkeletalMeshComponent>("MeshThirdPerson");
	MeshThirdPerson->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	MeshThirdPerson->bReceivesDecals = false;
	MeshThirdPerson->CastShadow = true;
	MeshThirdPerson->SetupAttachment(MeshFirstPerson);
	//MeshThirdPerson->SetHiddenInGame(true);
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
	AMFPSCharacter* FPSCharacter = Cast<AMFPSCharacter>(GetOwner());
	if (IsValid(FPSCharacter))
	{
		
	}
}

void AFPSWeapon::BeginPlay()
{
	Super::BeginPlay();
}

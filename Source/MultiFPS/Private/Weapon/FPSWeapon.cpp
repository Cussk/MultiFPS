// Copyright Kyle Cuss and Cuss Programming


#include "Weapon/FPSWeapon.h"

#include "KismetTraceUtils.h"
#include "Character/MFPSCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetArrayLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MultiFPS/MultiFPS.h"

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
	
	AimFOV = 65.0f;
	TraceRadius = 5.0f;
	FireTime = 0.1f;
	MagCapacity = 10;
	Ammo = 5;
	StartingCarriedAmmo = 10;
	Sequence = 0;
}

void AFPSWeapon::OnRep_Instigator()
{
	Super::OnRep_Instigator();
	RefreshWeaponPresentation();
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

UMaterialInstanceDynamic* AFPSWeapon::GetReticleMaterialInstance()
{
	if (!IsValid(ReticleMaterialInstance))
	{
		ReticleMaterialInstance = UMaterialInstanceDynamic::Create(ReticleMaterial, this);
	}
	
	return ReticleMaterialInstance;
}

UMaterialInstanceDynamic* AFPSWeapon::GetAmmoCounterMaterialInstance()
{
	if (!IsValid(AmmoCounterMaterialInstance))
	{
		AmmoCounterMaterialInstance = UMaterialInstanceDynamic::Create(AmmoCounterMaterial, this);
	}
	
	return AmmoCounterMaterialInstance;
}

void AFPSWeapon::SetEquippedPresentation(bool bEquipped)
{
	bShouldBeVisibleAsEquipped = bEquipped;
	RefreshWeaponPresentation();
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

void AFPSWeapon::RefreshWeaponPresentation() const
{
	APawn* OwningPawn = GetInstigator();

	if (!IsValid(OwningPawn) ||
		!OwningPawn->Implements<UPlayerInterface>())
	{
		HideMeshes();
		return;
	}

	AttachToOwningPawn();

	if (bShouldBeVisibleAsEquipped)
	{
		SetMeshVisibilities(OwningPawn);
	}
	else
	{
		HideMeshes();
	}
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

void AFPSWeapon::WeaponTrace(FHitResult& OutHit, float TraceLength)
{
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(this);
	
	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetAllChannels(ECR_Ignore);
	ResponseParams.CollisionResponse.SetResponse(ECC_Pawn, ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECC_WorldStatic, ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECC_WorldDynamic, ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECC_PhysicsBody, ECR_Block);
	
	ensure(GetInstigator());
	if (APlayerController* PC = Cast<APlayerController>(GetInstigator()->GetController()); IsValid(PC))
	{
		FVector EyesWorldLocation;
		FRotator EyesWorldRotation;
		PC->GetActorEyesViewPoint(EyesWorldLocation, EyesWorldRotation);
		const FVector EyesWorldDirection = UKismetMathLibrary::GetForwardVector(EyesWorldRotation);
		
		const FVector TraceStart = EyesWorldLocation;
		const FVector TraceEnd = TraceStart + EyesWorldDirection * TraceLength;
		
		const bool bHit = GetWorld()->SweepSingleByChannel(
			OutHit, 
			TraceStart, 
			TraceEnd, 
			FQuat::Identity, 
			MFPSTraceChannels::ECC_Weapon,
			FCollisionShape::MakeSphere(TraceRadius), 
			QueryParams, 
			ResponseParams);
		
		if (!bHit)
		{
			OutHit.ImpactPoint = TraceEnd;
		}
	}
}

void AFPSWeapon::Local_Fire(const FVector& ImpactPoint, const FVector& ImpactNormal,
	TEnumAsByte<EPhysicalSurface> ImpactSurfaceType, bool bIsFirstPerson)
{
	FireEffects(ImpactPoint, ImpactNormal, ImpactSurfaceType, bIsFirstPerson);
	
	if (GetInstigator()->IsLocallyControlled())
	{
		Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
		++Sequence;
	}
}

void AFPSWeapon::Auth_Fire()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
}

void AFPSWeapon::Rep_Fire(int32 AuthAmmo)
{
	if (GetInstigator()->IsLocallyControlled())
	{
		Ammo = AuthAmmo;
		--Sequence;
		Ammo -= Sequence;
	}
}

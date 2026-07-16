// Copyright Kyle Cuss and Cuss Programming


#include "Components/CombatComponent.h"

#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/WeaponData.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/PlayerInterface.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Weapon/FPSWeapon.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	TraceLength = 20'000;
	bAiming = false;
	bTriggerPressed = false;
}

void UCombatComponent:: GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UCombatComponent, Inventory);
	DOREPLIFETIME(UCombatComponent, CurrentWeapon);
	DOREPLIFETIME_CONDITION(UCombatComponent, bAiming, COND_SkipOwner);
}

void UCombatComponent::InitiateCycleWeapon()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("InitiateCycleWeapon"), false);
}

void UCombatComponent::InitiateFireWeapon_Pressed()
{
	if (!IsValid(CurrentWeapon))
	{
		return;
	}
	
	bTriggerPressed = true;
	
	if (CurrentWeapon->Ammo <= 0)
	{
		Local_DryFireWeapon();
		return;
	}
	
	Local_FireWeapon();
}

void UCombatComponent::InitiateFireWeapon_Released()
{
	bTriggerPressed = false;
}

void UCombatComponent::InitiateReloadWeapon()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("InitiateReloadWeapon"), false);
}

void UCombatComponent::InitiateAim_Pressed()
{
	Local_Aim(true);
	Server_Aim(true);
}

void UCombatComponent::InitiateAim_Released()
{
	Local_Aim(false);
	Server_Aim(false);
}

void UCombatComponent::Server_Aim_Implementation(const bool bPressed)
{
	Local_Aim(bPressed);
}

void UCombatComponent::Server_FireWeapon_Implementation(const FHitResult& Hit)
{
	if (!IsValid(CurrentWeapon))
	{
		return;
	}
	
	if (GetNetMode() != NM_ListenServer || !Cast<APawn>(GetOwner())->IsLocallyControlled())
	{
		CurrentWeapon->Auth_Fire();	
	}
	
	Multicast_FireWeapon(Hit, CurrentWeapon->Ammo);
}

void UCombatComponent::Server_DryFireWeapon_Implementation()
{
	if (!IsValid(CurrentWeapon))
	{
		return;
	}
	
	Multicast_DryFireWeapon();
}

void UCombatComponent::Local_Aim(const bool bPressed)
{
	bAiming = bPressed;
	OnAiming.Broadcast(bPressed);
}

void UCombatComponent::Local_FireWeapon()
{
	if (!IsValid(CurrentWeapon))
	{
		return;
	}
	
	ensure(IsValid(WeaponData));
	
	UAnimMontage* MontageFirstPerson = WeaponData->FirstPersonMontages.FindChecked(CurrentWeapon->WeaponTypeTag).FireMontage;
	const USkeletalMeshComponent* MeshFirstPerson = IPlayerInterface::Execute_GetMeshFirstPerson(GetOwner());
	if (IsValid(MontageFirstPerson) && IsValid(MeshFirstPerson))
	{
		MeshFirstPerson->GetAnimInstance()->Montage_Play(MontageFirstPerson, 1.0f);
	}
	
	FHitResult HitResult;
	CurrentWeapon->WeaponTrace(HitResult, TraceLength);
	
	EPhysicalSurface ImpactSurfaceType = HitResult.PhysMaterial.IsValid(false) ? HitResult.PhysMaterial->SurfaceType.GetValue() : SurfaceType1;
	CurrentWeapon->Local_Fire(HitResult.ImpactPoint, HitResult.ImpactNormal, ImpactSurfaceType, true);
	
	OnRoundFired.Broadcast(CurrentWeapon->Ammo, CurrentWeapon->MagCapacity);
	
	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &ThisClass::FireTimerFinished, CurrentWeapon->FireTime);
	Server_FireWeapon(HitResult);
}

void UCombatComponent::Local_DryFireWeapon()
{
	if (!IsValid(CurrentWeapon))
	{
		return;
	}
	
	ensure(IsValid(WeaponData));
	
	UAnimMontage* MontageFirstPerson = WeaponData->FirstPersonMontages.FindChecked(CurrentWeapon->WeaponTypeTag).DryFireMontage;
	const USkeletalMeshComponent* MeshFirstPerson = IPlayerInterface::Execute_GetMeshFirstPerson(GetOwner());
	if (IsValid(MontageFirstPerson) && IsValid(MeshFirstPerson))
	{
		MeshFirstPerson->GetAnimInstance()->Montage_Play(MontageFirstPerson, 1.0f);
	}
	
	Server_DryFireWeapon();
}

void UCombatComponent::Multicast_FireWeapon_Implementation(const FHitResult& Hit, int32 AuthAmmo)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	
	if (const APawn* OwningPawn = Cast<APawn>(GetOwner()); OwningPawn->IsLocallyControlled())
	{
		CurrentWeapon->Rep_Fire(AuthAmmo);
	}
	else
	{
		EPhysicalSurface ImpactSurfaceType = Hit.PhysMaterial.IsValid(false) ? Hit.PhysMaterial->SurfaceType.GetValue() : SurfaceType1;
		CurrentWeapon->Local_Fire(Hit.ImpactPoint, Hit.ImpactNormal, ImpactSurfaceType, false);
		
		ensure(IsValid(WeaponData));
        	
        	UAnimMontage* MontageThirdPerson = WeaponData->ThirdPersonMontages.FindChecked(CurrentWeapon->WeaponTypeTag).FireMontage;
        	const USkeletalMeshComponent* MeshThirdPerson = IPlayerInterface::Execute_GetMeshThirdPerson(GetOwner());
        	if (IsValid(MontageThirdPerson) && IsValid(MeshThirdPerson))
        	{
        		MeshThirdPerson->GetAnimInstance()->Montage_Play(MontageThirdPerson, 1.0f);
        	}
	}
}

void UCombatComponent::Multicast_DryFireWeapon_Implementation()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	
	CurrentWeapon->DryFireEffects();
	
	if (const APawn* OwningPawn = Cast<APawn>(GetOwner()); OwningPawn->IsLocallyControlled())
	{
		return;
	}
	
	ensure(IsValid(WeaponData));
        	
	UAnimMontage* MontageThirdPerson = WeaponData->ThirdPersonMontages.FindChecked(CurrentWeapon->WeaponTypeTag).DryFireMontage;
	const USkeletalMeshComponent* MeshThirdPerson = IPlayerInterface::Execute_GetMeshThirdPerson(GetOwner());
	if (IsValid(MontageThirdPerson) && IsValid(MeshThirdPerson))
	{
		MeshThirdPerson->GetAnimInstance()->Montage_Play(MontageThirdPerson, 1.0f);
	}
	
	CurrentWeapon->DryFireEffects();
}

void UCombatComponent::EquipWeapon(AFPSWeapon* Weapon)
{
	if (!GetOwner()->HasAuthority() || !IsValid(Weapon) || Weapon == CurrentWeapon)
	{
		return;
	}

	AFPSWeapon* PreviousWeapon = CurrentWeapon;
	CurrentWeapon = Weapon;

	HandleCurrentWeaponChanged(PreviousWeapon);
}

void UCombatComponent::SpawnInventory()
{
	if (GetOwner()->GetLocalRole() < ROLE_Authority)
	{
		return;
	}
	
	for (const TSubclassOf<AFPSWeapon>& WeaponClass : DefaultWeaponClasses)
	{
		AFPSWeapon* NewWeapon = SpawnWeapon(WeaponClass);
		Inventory.AddUnique(NewWeapon);
		NewWeapon->HideMeshes();
	}
	
	if (Inventory.Num() > 0)
	{
		EquipWeapon(Inventory[0]);
		InitializeWeaponWidgets();
	}
}

void UCombatComponent::DestroyInventory()
{
	for ( AFPSWeapon* Weapon : Inventory)
	{
		if (IsValid(Weapon))
		{
			Weapon->Destroy();
		}
	}
	Inventory.Empty();
}

void UCombatComponent::InitializeWeaponWidgets() const
{
	if (IsValid(CurrentWeapon))
	{
		OnReticleChanged.Broadcast(CurrentWeapon->GetReticleMaterialInstance());
		OnAmmoCounterChanged.Broadcast(CurrentWeapon->GetAmmoCounterMaterialInstance(), CurrentWeapon->Ammo, CurrentWeapon->MagCapacity);
	}
}

AFPSWeapon* UCombatComponent::SpawnWeapon(TSubclassOf<AFPSWeapon> WeaponClass) const
{
	AActor* OwningActor = GetOwner();
	if (!IsValid(OwningActor))
	{
		return nullptr;
	}
	
	if (OwningActor->GetLocalRole() < ROLE_Authority)
	{
		return nullptr;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = Cast<APawn>(OwningActor);
	SpawnParams.Owner = OwningActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	return GetWorld()->SpawnActor<AFPSWeapon>(WeaponClass, SpawnParams);
}

void UCombatComponent::HandleCurrentWeaponChanged(AFPSWeapon* LastWeapon) const
{
	if (IsValid(LastWeapon))
	{
		LastWeapon->SetEquippedPresentation(false);
	}

	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->SetEquippedPresentation(true);
		InitializeWeaponWidgets();
	}
}

void UCombatComponent::FireTimerFinished()
{
	if (!IsValid(CurrentWeapon))
	{
		return;
	}
	
	if (bTriggerPressed && CurrentWeapon->FireType == EFireType::Auto)
	{
		if (CurrentWeapon->Ammo <= 0)
		{
			Local_DryFireWeapon();
			bTriggerPressed = false;
			return;
		}
		
		Local_FireWeapon();
	}
}

void UCombatComponent::OnRep_CurrentWeapon(AFPSWeapon* LastWeapon) const
{
	HandleCurrentWeaponChanged(LastWeapon);
	IPlayerInterface::Execute_WeaponReplicated(GetOwner());
}

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
#include "Kismet/KismetMathLibrary.h"
#include "MultiFPS/MultiFPS.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Weapon/MFPSWeapon.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	TraceLength = 20'000;
	bAiming = false;
	bTriggerPressed = false;
	Local_WeaponIndex = 0;
}

void UCombatComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwningPawn) || !OwningPawn->IsLocallyControlled())
	{
		return;
	}
	
	APlayerController* PC = Cast<APlayerController>(OwningPawn->GetController());
	if (!IsValid(PC))
	{
		return;
	}
	
	FVector EyesWorldLocation;
	FRotator EyesWorldRotation;
	PC->GetActorEyesViewPoint(EyesWorldLocation, EyesWorldRotation);
	const FVector EyesWorldDirection = UKismetMathLibrary::GetForwardVector(EyesWorldRotation);
	
	const FVector TraceStart = EyesWorldLocation;
	const FVector TraceEnd = TraceStart + EyesWorldDirection * TraceLength;
	
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	
	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetAllChannels(ECR_Ignore);
	ResponseParams.CollisionResponse.SetResponse(ECC_Pawn, ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECC_PhysicsBody, ECR_Block);
	
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, MFPSTraceChannels::ECC_Weapon, QueryParams, ResponseParams);
	
	bHitPlayer = IsValid(HitResult.GetActor()) && HitResult.GetActor()->Implements<UPlayerInterface>();
	
	if (bHitPlayer != bHitPlayerLastFrame)
	{
		OnTargetingPlayerStatusChanged.Broadcast(bHitPlayer);
	}
	
	bHitPlayerLastFrame = bHitPlayer;
}

void UCombatComponent:: GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UCombatComponent, Inventory);
	DOREPLIFETIME(UCombatComponent, CurrentWeapon);
	DOREPLIFETIME_CONDITION(UCombatComponent, bAiming, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UCombatComponent, CurrentReserveAmmo, COND_OwnerOnly);
}

void UCombatComponent::InitiateCycleWeapon()
{
	if (!IsValid(CurrentWeapon))
	{
		return;
	}
	
	if (CurrentWeapon->WeaponStatus == EWeaponStatus::Cycling)
	{
		return;
	}
	
	AdvanceWeaponIndex();
	Local_CycleWeapon(Local_WeaponIndex);
	
}

void UCombatComponent::InitiateFireWeapon_Pressed()
{
	if (!IsValid(CurrentWeapon))
	{
		return;
	}
	
	bTriggerPressed = true;
	
	if (CurrentWeapon->WeaponStatus != EWeaponStatus::Idle)
	{
		return;
	}
	
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
	if (!IsValid(CurrentWeapon))
	{
		return;
	}
	
	if (CurrentWeapon->WeaponStatus == EWeaponStatus::Cycling || CurrentWeapon->WeaponStatus == EWeaponStatus::Reloading)
	{
		return;
	}
	
	if (CurrentWeapon->Ammo == CurrentWeapon->MagCapacity)
	{
		return;
	}
	
	if (CurrentReserveAmmo == 0)
	{
		return;
	}
	
	Local_ReloadWeapon();
	Server_ReloadWeapon();
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

void UCombatComponent::Notify_CycleWeapon()
{
	if (!IsValid(CurrentWeapon))
	{
		return;
	}
	
	AMFPSWeapon* NewWeapon = Inventory[Local_WeaponIndex];
	if (IsValid(NewWeapon))
	{
		Local_EquipWeapon(NewWeapon);
	}
}

void UCombatComponent::Notify_ReloadWeapon()
{
	if (!IsValid(CurrentWeapon))
	{
		return;
	}
	
	if (GetNetMode() != NM_Client)
	{
		const int32 EmptySpace = CurrentWeapon->MagCapacity - CurrentWeapon->Ammo;
		const int32 AmountToReload = FMath::Min(EmptySpace, CurrentReserveAmmo);
		CurrentWeapon->Ammo += AmountToReload;
		ReserveAmmo[CurrentWeapon->WeaponTypeTag] = ReserveAmmo[CurrentWeapon->WeaponTypeTag] - AmountToReload;
		CurrentReserveAmmo = ReserveAmmo[CurrentWeapon->WeaponTypeTag];
		
		Client_ReloadWeapon(CurrentWeapon->Ammo, CurrentReserveAmmo);
	}
	
	CurrentWeapon->WeaponStatus = EWeaponStatus::Idle;
	if (bTriggerPressed && CurrentWeapon->Ammo > 0)
	{
		Local_FireWeapon();
	}
}

void UCombatComponent::AddAmmo(const FGameplayTag WeaponType, int32 AmmoAmount)
{
	if (GetOwner()->HasAuthority() && !IsValid(CurrentWeapon))
	{
		return;
	}
	
	if (!ReserveAmmo.Contains(WeaponType))
	{
		ReserveAmmo.Add(WeaponType, AmmoAmount);
	}
	
	const int32 NewAmmo = ReserveAmmo.FindChecked(WeaponType) + AmmoAmount;
	ReserveAmmo[WeaponType] = NewAmmo;
	
	if (CurrentWeapon->WeaponTypeTag.MatchesTagExact(WeaponType))
	{
		CurrentReserveAmmo = NewAmmo;
		if (CurrentWeapon->Ammo == 0 && NewAmmo > 0)
		{
			Server_ReloadWeapon();
		}
		
		OnAmmoCounterChanged.Broadcast(CurrentWeapon->GetAmmoCounterMaterialInstance(), CurrentWeapon->Ammo, CurrentWeapon->MagCapacity);
		OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, CurrentWeapon->Ammo, CurrentWeapon->WeaponIcon);
	}
}

void UCombatComponent::Server_EquipWeapon_Implementation(AMFPSWeapon* Weapon)
{
	Local_EquipWeapon(Weapon);
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
	
	if (CurrentWeapon->Ammo <= 0)
	{
		return;
	}
	
	const bool bHit = IsValid(Hit.GetActor()) && Hit.GetActor()->Implements<UPlayerInterface>();
	const bool bHeadShot = Hit.BoneName == "head";
	bool bLethal = false;
	
	if (bHit)
	{
		bLethal = IPlayerInterface::Execute_DoDamage(Hit.GetActor(), CurrentWeapon->Damage, GetOwner());
	}
	
	FFiredRoundReport RoundReport;
	RoundReport.Attacker = GetOwner();
	RoundReport.Victim = Hit.GetActor();
	RoundReport.bHit = bHit;
	RoundReport.bHeadShot = bHeadShot;
	RoundReport.bLethal = bLethal;
	OnRoundReported.Broadcast(RoundReport);
	
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

void UCombatComponent::Server_CycleWeapon_Implementation(int32 WeaponIndex)
{
	Local_WeaponIndex = WeaponIndex;
	Multicast_CycleWeapon(WeaponIndex);
}

void UCombatComponent::Server_ReloadWeapon_Implementation()
{
	Multicast_ReloadWeapon();
}

void UCombatComponent::BlendOut_CycleWeapon(UAnimMontage* Montage, bool bInterrupted)
{
	UAnimInstance* AnimInstance = IPlayerInterface::Execute_GetMeshFirstPerson(GetOwner())->GetAnimInstance();
	if (IsValid(AnimInstance) && AnimInstance->OnMontageBlendingOut.IsAlreadyBound(this, &UCombatComponent::BlendOut_CycleWeapon))
	{
		AnimInstance->OnMontageBlendingOut.RemoveDynamic(this, &UCombatComponent::BlendOut_CycleWeapon);
	}
	
	if (!IsValid(CurrentWeapon))
	{
		return;
	}
	
	CurrentWeapon->WeaponStatus = EWeaponStatus::Idle;
	
	InitializeWeaponWidgets();
	
	if (bTriggerPressed && CurrentWeapon->Ammo > 0)
	{
		Local_FireWeapon();
	}
}

void UCombatComponent::BlendOut_DryFireWeapon(UAnimMontage* Montage, bool bInterrupted)
{
	UAnimInstance* AnimInstance = IPlayerInterface::Execute_GetMeshFirstPerson(GetOwner())->GetAnimInstance();
	if (IsValid(AnimInstance) && AnimInstance->OnMontageBlendingOut.IsAlreadyBound(this, &UCombatComponent::BlendOut_DryFireWeapon))
	{
		AnimInstance->OnMontageBlendingOut.RemoveDynamic(this, &UCombatComponent::BlendOut_DryFireWeapon);
	}
	
	if (!IsValid(CurrentWeapon))
	{
		return;
	}

	const APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (CurrentReserveAmmo > 0 && IsValid(OwningPawn) && OwningPawn->IsLocallyControlled())
	{
		Local_ReloadWeapon();
		Server_ReloadWeapon();
		return;
	}
	
	CurrentWeapon->WeaponStatus = EWeaponStatus::Idle;
}

void UCombatComponent::Local_EquipWeapon(AMFPSWeapon* Weapon)
{
	if (!IsValid(Weapon) || Weapon == CurrentWeapon)
	{
		return;
	}
	
	if (GetOwner()->HasAuthority())
	{
		SetCurrentWeapon(Weapon, CurrentWeapon);
	}
	else
	{
		Server_EquipWeapon(Weapon);
	}
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
	
	CurrentWeapon->WeaponStatus = EWeaponStatus::Firing;
	
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
	
	OnRoundFired.Broadcast(CurrentWeapon->Ammo, CurrentWeapon->MagCapacity, CurrentReserveAmmo);
	
	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &ThisClass::FireTimerFinished, CurrentWeapon->FireTime);
	Server_FireWeapon(HitResult);
}

void UCombatComponent::Local_DryFireWeapon()
{
	if (!IsValid(CurrentWeapon) || CurrentWeapon->WeaponStatus == EWeaponStatus::DryFiring)
	{
		return;
	}
	
	ensure(IsValid(WeaponData));
	
	CurrentWeapon->WeaponStatus = EWeaponStatus::DryFiring;
	
	UAnimMontage* MontageFirstPerson = WeaponData->FirstPersonMontages.FindChecked(CurrentWeapon->WeaponTypeTag).DryFireMontage;
	const USkeletalMeshComponent* MeshFirstPerson = IPlayerInterface::Execute_GetMeshFirstPerson(GetOwner());
	if (IsValid(MontageFirstPerson) && IsValid(MeshFirstPerson))
	{
		MeshFirstPerson->GetAnimInstance()->Montage_Play(MontageFirstPerson, 1.0f);
		MeshFirstPerson->GetAnimInstance()->OnMontageBlendingOut.AddDynamic(this, &UCombatComponent::BlendOut_DryFireWeapon);
	}
	
	Server_DryFireWeapon();
}

void UCombatComponent::Local_CycleWeapon(int32 WeaponIndex)
{
	AMFPSWeapon* NextWeapon = Inventory[WeaponIndex];
	if (!IsValid(NextWeapon) || !IsValid(WeaponData))
	{
		return;
	}
	
	CurrentWeapon->WeaponStatus = EWeaponStatus::Cycling;
	NextWeapon->WeaponStatus = EWeaponStatus::Cycling;
	
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	const bool bIsLocal = IsValid(OwningPawn) && OwningPawn->IsLocallyControlled();
	
	const FMontageData& MontageData = bIsLocal ? WeaponData->FirstPersonMontages.FindChecked(NextWeapon->WeaponTypeTag) : WeaponData->ThirdPersonMontages.FindChecked(NextWeapon->WeaponTypeTag);
	USkeletalMeshComponent* Mesh = bIsLocal ? IPlayerInterface::Execute_GetMeshFirstPerson(GetOwner()) : IPlayerInterface::Execute_GetMeshThirdPerson(GetOwner());
	if (IsValid(Mesh) && IsValid(MontageData.EquipMontage))
	{
		Mesh->GetAnimInstance()->Montage_Play(MontageData.EquipMontage, 1.0f);
	}
	
	if (bIsLocal)
	{
		Server_CycleWeapon(WeaponIndex);
		Mesh->GetAnimInstance()->OnMontageBlendingOut.AddDynamic(this, &UCombatComponent::BlendOut_CycleWeapon);
	}
}

void UCombatComponent::Local_ReloadWeapon()
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!IsValid(CurrentWeapon) | !IsValid(OwningPawn))
	{
		return;
	}
	ensure(WeaponData);
	
	const bool bIsLocal = OwningPawn->IsLocallyControlled();
	UAnimMontage* ReloadMontage = bIsLocal ? WeaponData->FirstPersonMontages.FindChecked(CurrentWeapon->WeaponTypeTag).ReloadMontage 
												: WeaponData->ThirdPersonMontages.FindChecked(CurrentWeapon->WeaponTypeTag).ReloadMontage;
	USkeletalMeshComponent* Mesh = bIsLocal ? IPlayerInterface::Execute_GetMeshFirstPerson(GetOwner()) : IPlayerInterface::Execute_GetMeshThirdPerson(GetOwner());
	if (IsValid(Mesh) && IsValid(ReloadMontage))
	{
		Mesh->GetAnimInstance()->Montage_Play(ReloadMontage, 1.0f);
	}
	
	UAnimMontage* WeaponReloadMontage =WeaponData->WeaponMontages.FindChecked(CurrentWeapon->WeaponTypeTag).ReloadMontage;
	USkeletalMeshComponent* WeaponMesh = bIsLocal ? CurrentWeapon->GetMeshFirstPerson() : CurrentWeapon->GetMeshThirdPerson();
	if (IsValid(WeaponMesh) && IsValid(WeaponReloadMontage))
	{
		WeaponMesh->GetAnimInstance()->Montage_Play(WeaponReloadMontage, 1.0f);
	}
	
	CurrentWeapon->WeaponStatus = EWeaponStatus::Reloading;
}


int32 UCombatComponent::AdvanceWeaponIndex()
{
	if (Inventory.Num() >= 2)
	{
		Local_WeaponIndex = (Local_WeaponIndex + 1) % Inventory.Num();
	}
	
	return Local_WeaponIndex;
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

void UCombatComponent::Multicast_CycleWeapon_Implementation(int32 WeaponIndex)
{
	
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwningPawn))
	{
		return;
	}
	
	if (!OwningPawn->IsLocallyControlled())
	{
		Local_WeaponIndex = WeaponIndex;
		Local_CycleWeapon(WeaponIndex);
	}
}

void UCombatComponent::Multicast_ReloadWeapon_Implementation()
{
	Local_ReloadWeapon();
}

void UCombatComponent::Client_ReloadWeapon_Implementation(int32 NewWeaponAmmo, int32 NewCarriedAmmo)
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!IsValid(CurrentWeapon) || !IsValid(OwningPawn))
	{
		return;
	}
	
	if (OwningPawn->IsLocallyControlled())
	{
		CurrentWeapon->Ammo = NewWeaponAmmo;
		CurrentReserveAmmo = NewCarriedAmmo;
		
		OnAmmoCounterChanged.Broadcast(CurrentWeapon->GetAmmoCounterMaterialInstance(), CurrentWeapon->Ammo, CurrentWeapon->MagCapacity);
		OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, CurrentWeapon->Ammo, CurrentWeapon->WeaponIcon);
	}
}

void UCombatComponent::SpawnInventory()
{
	if (GetOwner()->GetLocalRole() < ROLE_Authority)
	{
		return;
	}
	
	for (const TSubclassOf<AMFPSWeapon>& WeaponClass : DefaultWeaponClasses)
	{
		AMFPSWeapon* NewWeapon = SpawnWeapon(WeaponClass);
		Inventory.AddUnique(NewWeapon);
		ReserveAmmo.Add(NewWeapon->WeaponTypeTag, NewWeapon->StartingCarriedAmmo);
		NewWeapon->HideMeshes();
	}
	
	if (Inventory.Num() > 0)
	{
		Local_EquipWeapon(Inventory[0]);
		InitializeWeaponWidgets();
	}
}

void UCombatComponent::DestroyInventory()
{
	for ( AMFPSWeapon* Weapon : Inventory)
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
		OnReticleChanged.Broadcast(CurrentWeapon->GetReticleMaterialInstance(), CurrentWeapon->ReticleParams, bHitPlayer);
		OnAmmoCounterChanged.Broadcast(CurrentWeapon->GetAmmoCounterMaterialInstance(), CurrentWeapon->Ammo, CurrentWeapon->MagCapacity);
		OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, CurrentWeapon->Ammo, CurrentWeapon->WeaponIcon);
	}
}

AMFPSWeapon* UCombatComponent::SpawnWeapon(TSubclassOf<AMFPSWeapon> WeaponClass) const
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
	
	return GetWorld()->SpawnActor<AMFPSWeapon>(WeaponClass, SpawnParams);
}

void UCombatComponent::SetCurrentWeapon(AMFPSWeapon* NewWeapon, AMFPSWeapon* LastWeapon)
{
	AMFPSWeapon* LocalLastWeapon = nullptr;
	
	if (IsValid(LastWeapon))
	{
		LocalLastWeapon = LastWeapon;
		LastWeapon->SetEquippedPresentation(false);
	}
	else if (NewWeapon != CurrentWeapon) 
	{
		LocalLastWeapon = CurrentWeapon;
	}
	
	if (IsValid(LastWeapon))
	{
		LocalLastWeapon->DetachFromOwningPawn();
		LocalLastWeapon->WeaponStatus = EWeaponStatus::Unequipped;
	}
	
	CurrentWeapon = NewWeapon;
	
	if (!IsValid(CurrentWeapon))
	{
		return;
	}
	
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (IsValid(OwningPawn) && OwningPawn->HasAuthority())
	{
		CurrentReserveAmmo = ReserveAmmo.FindChecked(CurrentWeapon->WeaponTypeTag);
		OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, CurrentWeapon->Ammo, CurrentWeapon->WeaponIcon);
	}
	
	CurrentWeapon->SetEquippedPresentation(true);
	
	if (CurrentWeapon->Ammo == 0 && CurrentReserveAmmo > 0 && OwningPawn->IsLocallyControlled())
	{
		Local_ReloadWeapon();
		Server_ReloadWeapon();
	}
}

void UCombatComponent::FireTimerFinished()
{
	if (!IsValid(CurrentWeapon))
	{
		return;
	}
	
	if (CurrentWeapon->WeaponStatus == EWeaponStatus::Firing)
	{
		CurrentWeapon->WeaponStatus = EWeaponStatus::Idle;
	}
	
	if (bTriggerPressed && CurrentWeapon->FireType == EFireType::Auto && CurrentWeapon->WeaponStatus == EWeaponStatus::Idle)
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

void UCombatComponent::OnRep_CurrentReserveAmmo()
{
	if (IsValid(CurrentWeapon))
	{
		OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, CurrentWeapon->Ammo, CurrentWeapon->WeaponIcon);
	}
}

void UCombatComponent::OnRep_CurrentWeapon(AMFPSWeapon* LastWeapon)
{
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
	IPlayerInterface::Execute_WeaponReplicated(GetOwner());
}

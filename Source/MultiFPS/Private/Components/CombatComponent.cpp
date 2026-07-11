// Copyright Kyle Cuss and Cuss Programming


#include "Components/CombatComponent.h"

#include "Data/WeaponData.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/FPSWeapon.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("InitiateFireWeapon_Pressed"), false);
}

void UCombatComponent::InitiateFireWeapon_Released()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("InitiateFireWeapon_Released"), false);
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

void UCombatComponent::Local_Aim(bool bPressed)
{
	bAiming = bPressed;
	OnAiming.Broadcast(bPressed);
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
	}
}

void UCombatComponent::OnRep_CurrentWeapon(AFPSWeapon* LastWeapon) const
{
	HandleCurrentWeaponChanged(LastWeapon);
}

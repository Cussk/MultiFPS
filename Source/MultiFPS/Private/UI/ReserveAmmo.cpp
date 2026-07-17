// Copyright Kyle Cuss and Cuss Programming


#include "UI/ReserveAmmo.h"

#include "Character/MFPSCharacter.h"
#include "Components/CombatComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInterface.h"
#include "Weapon/MFPSWeapon.h"

void UReserveAmmo::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	WeaponIconImage->SetRenderOpacity(0.0f);
	AmmoText->SetRenderOpacity(0.0f);
	
	GetOwningPlayer()->OnPossessedPawnChanged.AddDynamic(this, &UReserveAmmo::OnPossessedPawnChanged);
	
	AMFPSCharacter* MFPSCharacter = Cast<AMFPSCharacter>(GetOwningPlayer()->GetPawn());
	if (!IsValid(MFPSCharacter))
	{
		return;
	}
	
	OnPossessedPawnChanged(nullptr, MFPSCharacter);
	
	if (MFPSCharacter->HasWeaponFirstReplicated())
	{
		AMFPSWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(MFPSCharacter);
		if (IsValid(Weapon))
		{
			OnCurrentReserveAmmoChanged(IPlayerInterface::Execute_GetCurrentWeaponReserveAmmo(MFPSCharacter), Weapon->Ammo, Weapon->WeaponIcon);
		}
	}
	else
	{
		MFPSCharacter->OnWeaponFirstReplicated.AddDynamic(this, &UReserveAmmo::OnWeaponFirstReplicated);
	}
	
	if (MFPSCharacter->HasAuthority())
	{
		AMFPSWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(MFPSCharacter);
		if (IsValid(Weapon))
		{
			OnCurrentReserveAmmoChanged(IPlayerInterface::Execute_GetCurrentWeaponReserveAmmo(MFPSCharacter), Weapon->Ammo, Weapon->WeaponIcon);
		}
	}
}

void UReserveAmmo::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	UCombatComponent* OldPawnCombatComponent = UCombatComponent::GetCombatComponent(OldPawn);
	if (IsValid(OldPawnCombatComponent))
	{
		OldPawnCombatComponent->OnCurrentReserveAmmoChanged.RemoveDynamic(this, &UReserveAmmo::OnCurrentReserveAmmoChanged);
		OldPawnCombatComponent->OnRoundFired.RemoveDynamic(this, &UReserveAmmo::OnRoundFired);
	}
	
	UCombatComponent* NewPawnCombatComponent = UCombatComponent::GetCombatComponent(NewPawn);
	if (IsValid(NewPawnCombatComponent))
	{
		WeaponIconImage->SetRenderOpacity(1.0f);
		AmmoText->SetRenderOpacity(1.0f);
		NewPawnCombatComponent->OnCurrentReserveAmmoChanged.AddDynamic(this, &UReserveAmmo::OnCurrentReserveAmmoChanged);
		NewPawnCombatComponent->OnRoundFired.AddDynamic(this, &UReserveAmmo::OnRoundFired);
	}
}

void UReserveAmmo::OnCurrentReserveAmmoChanged(int32 RoundInReserve, int32 RoundsInWeapon, UMaterialInterface* WeaponIconMaterial)
{
	if (IsValid(WeaponIconMaterial))
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(WeaponIconMaterial);
		if (IsValid(WeaponIconImage))
		{
			WeaponIconImage->SetBrush(Brush);
		}
	}
	
	if (IsValid(AmmoText))
	{
		FText CurrentAmmoText = FText::Format(NSLOCTEXT("AmmoText", "AmmoKey", "{0}/{1}"), RoundsInWeapon, RoundInReserve);
		AmmoText->SetText(CurrentAmmoText);
	}
}

void UReserveAmmo::OnRoundFired(int32 RoundsCurrent, int32 RoundsMax, int32 RoundsInReserve)
{
	if (IsValid(AmmoText))
	{
		FText CurrentAmmoText = FText::Format(NSLOCTEXT("AmmoText", "AmmoKey", "{0}/{1}"), RoundsCurrent, RoundsInReserve);
		AmmoText->SetText(CurrentAmmoText);
	}
}

void UReserveAmmo::OnWeaponFirstReplicated(AMFPSWeapon* Weapon, bool bIsTargetingPlayer)
{
	AMFPSCharacter* MFPSCharacter = Cast<AMFPSCharacter>(GetOwningPlayer()->GetPawn());
	if (!IsValid(MFPSCharacter))
	{
		return;
	}
	
	OnCurrentReserveAmmoChanged(IPlayerInterface::Execute_GetCurrentWeaponReserveAmmo(MFPSCharacter), Weapon->Ammo, Weapon->WeaponIcon);
}

// Copyright Kyle Cuss and Cuss Programming


#include "UI/MFPSReticle.h"

#include "Character/MFPSCharacter.h"
#include "Components/CombatComponent.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Weapon/FPSWeapon.h"

namespace Ammo
{
	const FName Rounds_Current_Name(TEXT("Rounds_Current"));
	const FName Rounds_Max_Name(TEXT("Rounds_Max"));
}

void UMFPSReticle::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	ReticleImage->SetRenderOpacity(0.0f);
	AmmoCounterImage->SetRenderOpacity(0.0f);
	
	GetOwningPlayer()->OnPossessedPawnChanged.AddDynamic(this, &UMFPSReticle::OnPossesedPawnChanged);
	
	AMFPSCharacter* MFPSCharacter = Cast<AMFPSCharacter>(GetOwningPlayer()->GetPawn());
	if (!IsValid(MFPSCharacter))
	{
		return;
	}
	
	OnPossesedPawnChanged(nullptr, MFPSCharacter);
	
	if (MFPSCharacter->HasWeaponFirstReplicated())
	{
		AFPSWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(MFPSCharacter);
		if (IsValid(Weapon))
		{
			OnReticleChanged(Weapon->GetReticleMaterialInstance());
			OnAmmoCounterChanged(Weapon->GetAmmoCounterMaterialInstance(), Weapon->Ammo, Weapon->MagCapacity);
		}
	}
	else
	{
		MFPSCharacter->OnWeaponFirstReplicated.AddDynamic(this, &UMFPSReticle::OnWeaponFirstReplicated);
	}
	
	if (MFPSCharacter->HasAuthority())
	{
		AFPSWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(MFPSCharacter);
		if (IsValid(Weapon))
		{
			OnReticleChanged(Weapon->GetReticleMaterialInstance());
			OnAmmoCounterChanged(Weapon->GetAmmoCounterMaterialInstance(), Weapon->Ammo, Weapon->MagCapacity);
		}
	}
}

void UMFPSReticle::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UMFPSReticle::OnPossesedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	UCombatComponent* OldPawnCombatComponent = UCombatComponent::GetCombatComponent(OldPawn);
	if (IsValid(OldPawnCombatComponent))
	{
		OldPawnCombatComponent->OnReticleChanged.RemoveDynamic(this, &UMFPSReticle::OnReticleChanged);
		OldPawnCombatComponent->OnAmmoCounterChanged.RemoveDynamic(this, &UMFPSReticle::OnAmmoCounterChanged);
		OldPawnCombatComponent->OnRoundFired.RemoveDynamic(this, &UMFPSReticle::OnRoundFired);
	}
	
	UCombatComponent* NewPawnCombatComponent = UCombatComponent::GetCombatComponent(NewPawn);
	if (IsValid(NewPawnCombatComponent))
	{
		ReticleImage->SetRenderOpacity(1.0f);
		AmmoCounterImage->SetRenderOpacity(1.0f);
		NewPawnCombatComponent->OnReticleChanged.AddDynamic(this, &UMFPSReticle::OnReticleChanged);
		NewPawnCombatComponent->OnAmmoCounterChanged.AddDynamic(this, &UMFPSReticle::OnAmmoCounterChanged);
		NewPawnCombatComponent->OnRoundFired.AddDynamic(this, &UMFPSReticle::OnRoundFired);
	}
}

void UMFPSReticle::OnWeaponFirstReplicated(AFPSWeapon* Weapon)
{
	OnReticleChanged(Weapon->GetReticleMaterialInstance());
	OnAmmoCounterChanged(Weapon->GetAmmoCounterMaterialInstance(), Weapon->Ammo, Weapon->MagCapacity);
}

void UMFPSReticle::OnReticleChanged(UMaterialInstanceDynamic* ReticleDynamicMaterialInstance)
{
	CurrentReticle_DynamicMaterialInstance = ReticleDynamicMaterialInstance;
	
	FSlateBrush Brush;
	Brush.SetResourceObject(ReticleDynamicMaterialInstance);
	if (IsValid(ReticleImage))
	{
		ReticleImage->SetBrush(Brush);
	}
}

void UMFPSReticle::OnAmmoCounterChanged(UMaterialInstanceDynamic* AmmoCounterDynamicMaterialInstance, int32 RoundsCurrent,
	int32 RoundsMax)
{
	CurrentAmmoCounter_DynamicMaterialInstance = AmmoCounterDynamicMaterialInstance;
	CurrentAmmoCounter_DynamicMaterialInstance->SetScalarParameterValue(Ammo::Rounds_Current_Name, RoundsCurrent);
	CurrentAmmoCounter_DynamicMaterialInstance->SetScalarParameterValue(Ammo::Rounds_Max_Name, RoundsMax);
	
	FSlateBrush Brush;
	Brush.SetResourceObject(AmmoCounterDynamicMaterialInstance);
	if (IsValid(AmmoCounterImage))
	{
		AmmoCounterImage->SetBrush(Brush);
	}
}

void UMFPSReticle::OnRoundFired(int32 RoundsCurrent, int32 RoundsMax)
{
	if (CurrentAmmoCounter_DynamicMaterialInstance.IsValid())
	{
		CurrentAmmoCounter_DynamicMaterialInstance->SetScalarParameterValue(Ammo::Rounds_Current_Name, RoundsCurrent);
		CurrentAmmoCounter_DynamicMaterialInstance->SetScalarParameterValue(Ammo::Rounds_Max_Name, RoundsMax);
	}
}

// Copyright Kyle Cuss and Cuss Programming


#include "UI/MFPSReticle.h"

#include "Character/MFPSCharacter.h"
#include "Components/CombatComponent.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Weapon/MFPSWeapon.h"

namespace Ammo
{
	const FName Rounds_Current_Name(TEXT("Rounds_Current"));
	const FName Rounds_Max_Name(TEXT("Rounds_Max"));
}

namespace Reticle
{
	const FName RoundedCornerScale_Name(TEXT("RoundedCornerScale"));
	const FName ShapeCutThickness_Name(TEXT("ShapeCutThickness"));
	const FName Inner_RGBA_Name(TEXT("Inner_RGBA"));
}

void UMFPSReticle::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	ReticleImage->SetRenderOpacity(0.0f);
	AmmoCounterImage->SetRenderOpacity(0.0f);
	_BaseCornerScaleFactor_RoundFired = 0.0f;
	_BaseShapeCutFactor_RoundFired = 0.0f;
	_BaseCornerScaleFactor_Aiming = 0.0f;
	_BaseShapeCutFactor_Aiming = 0.0f;
	_BaseCornerScaleFactor_TargetingPlayer = 0.0f;
	bAiming = false;
	
	GetOwningPlayer()->OnPossessedPawnChanged.AddDynamic(this, &UMFPSReticle::OnPossessedPawnChanged);
	
	AMFPSCharacter* MFPSCharacter = Cast<AMFPSCharacter>(GetOwningPlayer()->GetPawn());
	if (!IsValid(MFPSCharacter))
	{
		return;
	}

	const UCombatComponent* CombatComponent = MFPSCharacter->GetCombatComponent();
	if (!IsValid(CombatComponent))
	{
		return;
	}
	
	OnPossessedPawnChanged(nullptr, MFPSCharacter);
	
	if (MFPSCharacter->HasWeaponFirstReplicated())
	{
		AMFPSWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(MFPSCharacter);
		if (IsValid(Weapon))
		{
			OnReticleChanged(Weapon->GetReticleMaterialInstance(), Weapon->ReticleParams, CombatComponent->bHitPlayer);
			OnAmmoCounterChanged(Weapon->GetAmmoCounterMaterialInstance(), Weapon->Ammo, Weapon->MagCapacity);
		}
	}
	else
	{
		MFPSCharacter->OnWeaponFirstReplicated.AddDynamic(this, &UMFPSReticle::OnWeaponFirstReplicated);
	}
	
	if (MFPSCharacter->HasAuthority())
	{
		AMFPSWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(MFPSCharacter);
		if (IsValid(Weapon))
		{
			OnReticleChanged(Weapon->GetReticleMaterialInstance(), Weapon->ReticleParams, CombatComponent->bHitPlayer);
			OnAmmoCounterChanged(Weapon->GetAmmoCounterMaterialInstance(), Weapon->Ammo, Weapon->MagCapacity);
		}
	}
}

void UMFPSReticle::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	_BaseCornerScaleFactor_RoundFired = FMath::FInterpTo(_BaseCornerScaleFactor_RoundFired, 0.0f, InDeltaTime, CurrentReticleParams.RoundFiredInterpSpeed);
	_BaseShapeCutFactor_RoundFired = FMath::FInterpTo(_BaseShapeCutFactor_RoundFired, 0.0f, InDeltaTime, CurrentReticleParams.RoundFiredInterpSpeed);
	
	_BaseCornerScaleFactor_Aiming = FMath::FInterpTo(_BaseCornerScaleFactor_Aiming, bAiming ? CurrentReticleParams.ScaleFactor_Aiming : CurrentReticleParams.ScaleFactor_Default, 
		InDeltaTime, CurrentReticleParams.AimingInterpSpeed);
	
	_BaseShapeCutFactor_Aiming = FMath::FInterpTo(_BaseShapeCutFactor_Aiming, bAiming ? CurrentReticleParams.ShapeCutFactor_Aiming : CurrentReticleParams.ShapeCutFactor_Default, 
		InDeltaTime, CurrentReticleParams.AimingInterpSpeed);
	
	_BaseCornerScaleFactor_TargetingPlayer = FMath::FInterpTo(_BaseCornerScaleFactor_TargetingPlayer, bTargetingPlayer ? CurrentReticleParams.ScaleFactor_Targeting : CurrentReticleParams.ScaleFactor_NotTargeting,
		InDeltaTime, CurrentReticleParams.TargetingInterpSpeed);
	
	BaseCornerScaleFactor = _BaseCornerScaleFactor_RoundFired + _BaseCornerScaleFactor_Aiming + _BaseCornerScaleFactor_TargetingPlayer;
	BaseShapeCutFactor = _BaseShapeCutFactor_RoundFired + _BaseShapeCutFactor_Aiming;
	
	if (CurrentReticle_DynamicMaterialInstance.IsValid())
	{
		CurrentReticle_DynamicMaterialInstance->SetScalarParameterValue(Reticle::RoundedCornerScale_Name, BaseCornerScaleFactor);
		CurrentReticle_DynamicMaterialInstance->SetScalarParameterValue(Reticle::ShapeCutThickness_Name, BaseShapeCutFactor);
	}
}

void UMFPSReticle::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	UCombatComponent* OldPawnCombatComponent = UCombatComponent::GetCombatComponent(OldPawn);
	if (IsValid(OldPawnCombatComponent))
	{
		OldPawnCombatComponent->OnReticleChanged.RemoveDynamic(this, &UMFPSReticle::OnReticleChanged);
		OldPawnCombatComponent->OnAmmoCounterChanged.RemoveDynamic(this, &UMFPSReticle::OnAmmoCounterChanged);
		OldPawnCombatComponent->OnRoundFired.RemoveDynamic(this, &UMFPSReticle::OnRoundFired);
		OldPawnCombatComponent->OnAiming.RemoveDynamic(this, &UMFPSReticle::OnAiming);
		OldPawnCombatComponent->OnTargetingPlayerStatusChanged.RemoveDynamic(this, &UMFPSReticle::OnTargetingPlayerStatusChanged);
	}
	
	UCombatComponent* NewPawnCombatComponent = UCombatComponent::GetCombatComponent(NewPawn);
	if (IsValid(NewPawnCombatComponent))
	{
		ReticleImage->SetRenderOpacity(1.0f);
		AmmoCounterImage->SetRenderOpacity(1.0f);
		NewPawnCombatComponent->OnReticleChanged.AddDynamic(this, &UMFPSReticle::OnReticleChanged);
		NewPawnCombatComponent->OnAmmoCounterChanged.AddDynamic(this, &UMFPSReticle::OnAmmoCounterChanged);
		NewPawnCombatComponent->OnRoundFired.AddDynamic(this, &UMFPSReticle::OnRoundFired);
		NewPawnCombatComponent->OnAiming.AddDynamic(this, &UMFPSReticle::OnAiming);
		NewPawnCombatComponent->OnTargetingPlayerStatusChanged.AddDynamic(this, &UMFPSReticle::OnTargetingPlayerStatusChanged);
	}
}

void UMFPSReticle::OnWeaponFirstReplicated(AMFPSWeapon* Weapon, bool bIsTargetingPlayer)
{
	OnReticleChanged(Weapon->GetReticleMaterialInstance(), Weapon->ReticleParams, bIsTargetingPlayer);
	OnAmmoCounterChanged(Weapon->GetAmmoCounterMaterialInstance(), Weapon->Ammo, Weapon->MagCapacity);
}

void UMFPSReticle::OnReticleChanged(UMaterialInstanceDynamic* ReticleDynamicMaterialInstance, const FReticleParams& ReticleParams, bool bCurrentlyTargetingPlayer)
{
	CurrentReticle_DynamicMaterialInstance = ReticleDynamicMaterialInstance;
	CurrentReticleParams = ReticleParams;
	
	FSlateBrush Brush;
	Brush.SetResourceObject(ReticleDynamicMaterialInstance);
	if (IsValid(ReticleImage))
	{
		ReticleImage->SetBrush(Brush);
	}
	
	OnTargetingPlayerStatusChanged(bCurrentlyTargetingPlayer);
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

void UMFPSReticle::OnRoundFired(int32 RoundsCurrent, int32 RoundsMax, int32 RoundsInReserve)
{
	_BaseCornerScaleFactor_RoundFired += CurrentReticleParams.ScaleFactor_RoundFired;
	_BaseShapeCutFactor_RoundFired += CurrentReticleParams.ShapeCutFactor_RoundFired;
	
	if (CurrentAmmoCounter_DynamicMaterialInstance.IsValid())
	{
		CurrentAmmoCounter_DynamicMaterialInstance->SetScalarParameterValue(Ammo::Rounds_Current_Name, RoundsCurrent);
		CurrentAmmoCounter_DynamicMaterialInstance->SetScalarParameterValue(Ammo::Rounds_Max_Name, RoundsMax);
	}
}

void UMFPSReticle::OnAiming(bool IsAiming)
{
	bAiming = IsAiming;
}

void UMFPSReticle::OnTargetingPlayerStatusChanged(bool TargetPlayerChanged)
{
	bTargetingPlayer = TargetPlayerChanged;
	if (CurrentReticle_DynamicMaterialInstance.IsValid())
	{
		FLinearColor ReticleColor = bTargetingPlayer ? FLinearColor::Red : FLinearColor::White;
		CurrentReticle_DynamicMaterialInstance->SetVectorParameterValue(Reticle::Inner_RGBA_Name, ReticleColor);
	}
}

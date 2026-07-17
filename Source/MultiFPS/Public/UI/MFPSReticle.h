// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/MFPSTypes.h"
#include "MFPSReticle.generated.h"

class AFPSWeapon;
class UImage;
class UMaterialInstanceDynamic;

UCLASS()
class MULTIFPS_API UMFPSReticle : public UUserWidget
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ReticleImage;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> AmmoCounterImage;
	
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
private:
	TWeakObjectPtr<UMaterialInstanceDynamic> CurrentReticle_DynamicMaterialInstance;
	TWeakObjectPtr<UMaterialInstanceDynamic> CurrentAmmoCounter_DynamicMaterialInstance;
	FReticleParams CurrentReticleParams;
	float BaseCornerScaleFactor;
	float BaseShapeCutFactor;
	float _BaseCornerScaleFactor_RoundFired;
	float _BaseShapeCutFactor_RoundFired;
	float _BaseCornerScaleFactor_Aiming;
	float _BaseShapeCutFactor_Aiming;
	float _BaseCornerScaleFactor_TargetingPlayer;
	bool bAiming;
	bool bTargetingPlayer;
	
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
	UFUNCTION()
	void OnWeaponFirstReplicated(AFPSWeapon* Weapon);
	
	UFUNCTION()
	void OnReticleChanged(UMaterialInstanceDynamic* ReticleDynamicMaterialInstance, const FReticleParams& ReticleParams, bool bCurrentlyTargetingPlayer);
	
	UFUNCTION()
	void OnAmmoCounterChanged(UMaterialInstanceDynamic* AmmoCounterDynamicMaterialInstance, int32 RoundsCurrent, int32 RoundsMax);
	
	UFUNCTION()
	void OnRoundFired(int32 RoundsCurrent, int32 RoundsMax);
	
	UFUNCTION()
	void OnAiming(bool IsAiming);
	
	UFUNCTION()
	void OnTargetingPlayerStatusChanged(bool TargetPlayerChanged);
};

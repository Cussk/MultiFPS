// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReserveAmmo.generated.h"

class AMFPSWeapon;
class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class MULTIFPS_API UReserveAmmo : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
protected:
	virtual void NativeOnInitialized() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> WeaponIconImage;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AmmoText;
	
private:
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
	UFUNCTION()
	void OnCurrentReserveAmmoChanged(int32 RoundInReserve, int32 RoundsInWeapon, UMaterialInterface* WeaponIconMaterial);
	
	UFUNCTION()
	void OnRoundFired(int32 RoundsCurrent, int32 RoundsMax, int32 RoundsInReserve);
	
	UFUNCTION()
	void OnWeaponFirstReplicated(AMFPSWeapon* Weapon, bool bIsTargetingPlayer);
};

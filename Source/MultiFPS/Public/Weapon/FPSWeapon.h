// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "FPSWeapon.generated.h"

enum class EFireType : uint8;
enum EPhysicalSurface : int;

UCLASS()
class MULTIFPS_API AFPSWeapon : public AActor
{
	GENERATED_BODY()

public:
	AFPSWeapon();
	virtual void OnRep_Instigator() override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void DryFireEffects();
	
	USkeletalMeshComponent* GetMeshFirstPerson() const;
	USkeletalMeshComponent* GetMeshThirdPerson() const;
	
	void SetEquippedPresentation(bool bEquipped);
	
	void AttachToOwningPawn() const;
	void HideMeshes() const;
	
	void WeaponTrace(FHitResult& OutHit, float TraceLength);
	void Local_Fire(const FVector& ImpactPoint, const FVector& ImpactNormal, TEnumAsByte<EPhysicalSurface> ImpactSurfaceType, bool bIsFirstPerson);
	void Auth_Fire();
	void Rep_Fire(int32 AuthAmmo);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MFPS|WeaponType")
	FGameplayTag WeaponTypeTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MFPS|Aiming")
	float AimFOV = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="MFPS|Trace")
	float TraceRadius = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MFPS|FireType")
	EFireType FireType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MFPS|FireType")
	float FireTime;
	
	UPROPERTY(EditAnywhere, Category = "MFPS|Ammo")
	int32 MagCapacity;
	
	UPROPERTY(EditAnywhere, Category = "MFPS|Ammo")
	int32 Ammo;
	
	UPROPERTY(EditAnywhere, Category = "MFPS|Ammo")
	int32 StartingCarriedAmmo;	

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void FireEffects(const FVector& ImpactPoint, const FVector& ImpactNormal,
	EPhysicalSurface ImpactSurfaceType, bool bIsFirstPerson);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MFPS|Weapon")
	TObjectPtr<USkeletalMeshComponent> MeshFirstPerson;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MFPS|Weapon")
	TObjectPtr<USkeletalMeshComponent> MeshThirdPerson;

private:	
	void SetMeshVisibilities(const APawn* OwningPawn) const;
	void RefreshWeaponPresentation() const;
	
	bool bShouldBeVisibleAsEquipped = false;
	
	int32 Sequence;
};

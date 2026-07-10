// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "FPSWeapon.generated.h"

UCLASS()
class MULTIFPS_API AFPSWeapon : public AActor
{
	GENERATED_BODY()

public:
	AFPSWeapon();
	virtual void OnRep_Instigator() override;
	
	USkeletalMeshComponent* GetMeshFirstPerson() const;
	USkeletalMeshComponent* GetMeshThirdPerson() const;
	
	void AttachToOwningPawn() const;
	void HideMeshes() const;
	
	UPROPERTY(EditAnywhere, Category="MFPS|WeaponType")
	FGameplayTag WeaponTypeTag;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> MeshFirstPerson;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> MeshThirdPerson;
	
	void SetMeshVisibilities(const APawn* OwningPawn) const;
};

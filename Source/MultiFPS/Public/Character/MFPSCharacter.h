// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/PlayerInterface.h"
#include "MFPSCharacter.generated.h"

class UCombatComponent;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class MULTIFPS_API AMFPSCharacter : public ACharacter, public IPlayerInterface
{
	GENERATED_BODY()

public:
	AMFPSCharacter();
	
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	
	/** Player Interface */
	virtual FWeaponSocketAlignment GetTPWeaponSocketAlignment_Implementation(const FGameplayTag& WeaponType) const override;
	virtual FWeaponSocketAlignment GetFPWeaponSocketAlignment_Implementation(const FGameplayTag& WeaponType) const override;
	virtual USkeletalMeshComponent* GetMeshFirstPerson_Implementation() const override;
	virtual USkeletalMeshComponent* GetMeshThirdPerson_Implementation() const override;
	/** ~Player Interface */
	
	UFUNCTION(BlueprintCallable)
	UCombatComponent* GetCombatComponent();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MFPS|Components")
	TObjectPtr<UCombatComponent> CombatComponent;

private:	
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> MeshFirstPerson;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;
};

// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/PlayerInterface.h"
#include "MFPSCharacter.generated.h"

class AMFPSWeapon;
class UCombatComponent;
class UCameraComponent;
class USpringArmComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponFirstReplicated, AMFPSWeapon*, FirstWeapon, bool, bTargetingPlayer);

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
	virtual void OnRep_PlayerState() override;
	
	/** Player Interface */
	virtual FWeaponSocketAlignment GetTPWeaponSocketAlignment_Implementation(const FGameplayTag& WeaponType) const override;
	virtual FWeaponSocketAlignment GetFPWeaponSocketAlignment_Implementation(const FGameplayTag& WeaponType) const override;
	virtual USkeletalMeshComponent* GetMeshFirstPerson_Implementation() const override;
	virtual USkeletalMeshComponent* GetMeshThirdPerson_Implementation() const override;
	virtual AMFPSWeapon* GetCurrentWeapon_Implementation() const override;
	virtual int32 GetCurrentWeaponReserveAmmo_Implementation() const override;
	virtual void WeaponReplicated_Implementation() override;
	virtual void Notify_CycleWeapon_Implementation() override;
	virtual void Notify_ReloadWeapon_Implementation() override;
	virtual void AddAmmo_Implementation(const FGameplayTag WeaponType, int32 AmmoAmount) override;
	
	/** ~Player Interface */
	
	UFUNCTION(BlueprintCallable)
	UCombatComponent* GetCombatComponent();
	
	UFUNCTION(BlueprintPure)
	bool HasCurrentWeapon();
	
	UFUNCTION(BlueprintCallable)
	FRotator GetFixedAimRotation() const;
	
	bool HasWeaponFirstReplicated() const { return bWeaponFirstReplicated; }
	
	UPROPERTY(BlueprintReadOnly, Category = "MFPS|FABRIK")
	FTransform FABRIK_SocketTransform;
	
	UPROPERTY(BlueprintAssignable)
	FWeaponFirstReplicated OnWeaponFirstReplicated;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MFPS|Components")
	TObjectPtr<UCombatComponent> CombatComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MFPS|Camera")
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MFPS|Aiming")
	float DefaultFOV;
	
	UPROPERTY(BlueprintReadOnly, Category = "MFPS|TurnInPlace")
	float AOYaw;
	
	UPROPERTY(BlueprintReadOnly, Category = "MFPS|Strafing")
	float MovementOffsetYaw;
	
	UPROPERTY(BlueprintReadOnly, Category = "MFPS|TurnInPlace")
	ETurnInPlace TurningStatus;
	
	UPROPERTY(BlueprintReadOnly, Category = "MFPS|TurnInPlace")
	float TurnSpeed = 4.0f;

private:	
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> MeshFirstPerson;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	
	bool bWeaponFirstReplicated;
	FRotator StartingAimRotation;
	float InterpAOYaw;
	
	void CalculateFABRIKSocketTransforms();
	void CalculateTurnInPlaceParameters(float DeltaTime);
	
	void TurnInPlace(float DeltaTime);
};

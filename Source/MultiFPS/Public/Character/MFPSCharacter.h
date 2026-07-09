// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MFPSCharacter.generated.h"

class UCombatComponent;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class MULTIFPS_API AMFPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMFPSCharacter();
	
	virtual void Tick(float DeltaTime) override;
	
	UCombatComponent* GetCombatComponent();

protected:
	virtual void BeginPlay() override;

private:	
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCombatComponent> CombatComponent;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> MeshFirstPerson;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;
};

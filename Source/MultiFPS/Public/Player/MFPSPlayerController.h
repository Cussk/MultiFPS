// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MFPSPlayerController.generated.h"

class AMFPSCharacter;
class UCombatComponent;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class MULTIFPS_API AMFPSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMFPSPlayerController();
	virtual void AcknowledgePossession(APawn* P) override;
	virtual void OnRep_Pawn() override;
	
	bool bPawnAlive;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void SetupInputComponent() override;
	
private:
	UPROPERTY(EditAnywhere, Category="MFPS|Input")
	TObjectPtr<UInputMappingContext> FPSIMC;
	
	UPROPERTY(EditAnywhere, Category="MFPS|Input")
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditAnywhere, Category="MFPS|Input")
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditAnywhere, Category="MFPS|Input")
	TObjectPtr<UInputAction> JumpAction;
	
	UPROPERTY(EditAnywhere, Category="MFPS|Input")
	TObjectPtr<UInputAction> CrouchAction;
	
	UPROPERTY(EditAnywhere, Category="MFPS|Input")
	TObjectPtr<UInputAction> CycleWeaponAction;
	
	UPROPERTY(EditAnywhere, Category="MFPS|Input")
	TObjectPtr<UInputAction> AimWeaponAction;
	
	UPROPERTY(EditAnywhere, Category="MFPS|Input")
	TObjectPtr<UInputAction> FireWeaponAction;
	
	UPROPERTY(EditAnywhere, Category="MFPS|Input")
	TObjectPtr<UInputAction> ReloadWeaponAction;
	
	UPROPERTY()
	AMFPSCharacter* MFPSCharacter;
	
	UPROPERTY()
	UCombatComponent* CombatComponent;
	
	void SetupFPSCharacter(APawn* InPawn);
	
	void InputMove(const FInputActionValue& InputActionValue);
	void InputLook(const FInputActionValue& InputActionValue);
	void InputJump();
	void InputCrouch();
	void InputCycleWeapon();
	void InputReloadWeapon();
	void InputFireWeapon_Pressed();
	void InputFireWeapon_Released();
	void InputAimWeapon_Pressed();
	void InputAimWeapon_Released();
};

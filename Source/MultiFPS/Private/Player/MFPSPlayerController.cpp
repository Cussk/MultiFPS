// Copyright Kyle Cuss and Cuss Programming


#include "Player/MFPSPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/MFPSCharacter.h"
#include "Components/CombatComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"

AMFPSPlayerController::AMFPSPlayerController()
{
	bPawnAlive = true;
}

void AMFPSPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (IsValid(EnhancedInputSubsystem))
	{
		EnhancedInputSubsystem->AddMappingContext(FPSIMC, 0);
	}
}

void AMFPSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	SetupFPSCharacter(InPawn);
}

void AMFPSPlayerController::AcknowledgePossession(class APawn* P)
{
	Super::AcknowledgePossession(P);
	
	SetupFPSCharacter(P);
}

void AMFPSPlayerController::OnUnPossess()
{
	MFPSCharacter = nullptr;
	CombatComponent = nullptr;
	
	Super::OnUnPossess();
}

void AMFPSPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	
	SetupFPSCharacter(GetPawn());
}

void AMFPSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::InputMove);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::InputLook);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::InputJump);
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::InputCrouch);
	EnhancedInputComponent->BindAction(CycleWeaponAction, ETriggerEvent::Started, this, &ThisClass::InputCycleWeapon);
	EnhancedInputComponent->BindAction(ReloadWeaponAction, ETriggerEvent::Started, this, &ThisClass::InputReloadWeapon);
	EnhancedInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Started, this, &ThisClass::InputFireWeapon_Pressed);
	EnhancedInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Completed, this, &ThisClass::InputFireWeapon_Released);
	EnhancedInputComponent->BindAction(AimWeaponAction, ETriggerEvent::Started, this, &ThisClass::InputAimWeapon_Pressed);
	EnhancedInputComponent->BindAction(AimWeaponAction, ETriggerEvent::Completed, this, &ThisClass::InputAimWeapon_Released);
	
}

void AMFPSPlayerController::SetupFPSCharacter(APawn* InPawn)
{
	MFPSCharacter = Cast<AMFPSCharacter>(InPawn);
	if (IsValid(MFPSCharacter))
	{
		CombatComponent = MFPSCharacter->GetCombatComponent();
		bPawnAlive = true;
	}
}

void AMFPSPlayerController::InputMove(const FInputActionValue& InputActionValue)
{
	if (!bPawnAlive) return;
	
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
	
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
	
}

void AMFPSPlayerController::InputLook(const FInputActionValue& InputActionValue)
{
	if (!bPawnAlive) return;
	
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	
	AddYawInput(InputAxisVector.X);
	AddPitchInput(InputAxisVector.Y);
}

void AMFPSPlayerController::InputJump()
{
	if (!bPawnAlive) return;
	
	if (!IsValid(MFPSCharacter))
	{
		return;
	}
	
	UCharacterMovementComponent* CharacterMovementComponent = MFPSCharacter->GetCharacterMovement();
	if (!IsValid(CharacterMovementComponent))
	{
		return;
	}
	
	if (CharacterMovementComponent->bWantsToCrouch)
	{
		CharacterMovementComponent->bWantsToCrouch = false;
	}
	else
	{
		MFPSCharacter->Jump();
	}
}

void AMFPSPlayerController::InputCrouch()
{
	if (!bPawnAlive) return;
	
	if (!IsValid(MFPSCharacter))
	{
		return;
	}
	
	UCharacterMovementComponent* CharacterMovementComponent = MFPSCharacter->GetCharacterMovement();
	if (IsValid(CharacterMovementComponent))
	{
		CharacterMovementComponent->bWantsToCrouch = !CharacterMovementComponent->bWantsToCrouch;
	}
}

void AMFPSPlayerController::InputCycleWeapon()
{
	if (!bPawnAlive) return;
	
	if (CombatComponent)
	{
		CombatComponent->InitiateCycleWeapon();
	}
}

void AMFPSPlayerController::InputReloadWeapon()
{
	if (!bPawnAlive) return;
	
	if (CombatComponent)
	{
		CombatComponent->InitiateReloadWeapon();
	}
}

void AMFPSPlayerController::InputFireWeapon_Pressed()
{
	if (!bPawnAlive) return;
	
	if (CombatComponent)
	{
		CombatComponent->InitiateFireWeapon_Pressed();
	}
}

void AMFPSPlayerController::InputFireWeapon_Released()
{
	if (!bPawnAlive) return;
	
	if (CombatComponent)
	{
		CombatComponent->InitiateFireWeapon_Released();
	}
}

void AMFPSPlayerController::InputAimWeapon_Pressed()
{
	if (!bPawnAlive) return;
	
	if (CombatComponent)
	{
		CombatComponent->InitiateAim_Pressed();
	}
}

void AMFPSPlayerController::InputAimWeapon_Released()
{
	if (!bPawnAlive) return;
	
	if (CombatComponent)
	{
		CombatComponent->InitiateAim_Released();
	}
}

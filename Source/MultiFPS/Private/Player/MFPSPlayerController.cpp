// Copyright Kyle Cuss and Cuss Programming


#include "Player/MFPSPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AMFPSPlayerController::AMFPSPlayerController()
{

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
 
void AMFPSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::InputMove);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::InputLook);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::InputJump);
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::InputCrouch);
	
}

void AMFPSPlayerController::InputMove(const FInputActionValue& InputActionValue)
{
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
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	
	AddYawInput(InputAxisVector.X);
	AddPitchInput(InputAxisVector.Y);
}

void AMFPSPlayerController::InputJump()
{
	if (!IsValid(GetCharacter()))
	{
		return;
	}
	
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacter()->GetCharacterMovement();
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
		GetCharacter()->Jump();
	}
}

void AMFPSPlayerController::InputCrouch()
{
	if (!IsValid(GetCharacter()))
	{
		return;
	}
	
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacter()->GetCharacterMovement();
	if (IsValid(CharacterMovementComponent))
	{
		CharacterMovementComponent->bWantsToCrouch = !CharacterMovementComponent->bWantsToCrouch;
	}
}

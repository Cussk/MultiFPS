// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MULTIFPS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void InitiateCycleWeapon();
	void InitiateFireWeapon_Pressed();
	void InitiateFireWeapon_Released();
	void InitiateReloadWeapon();
	void InitiateAim_Pressed();
	void InitiateAim_Released();
	
protected:
	
	
private:
	
};

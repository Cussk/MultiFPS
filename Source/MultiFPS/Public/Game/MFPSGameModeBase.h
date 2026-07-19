// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MFPSGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class MULTIFPS_API AMFPSGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:	
	void RequestRespawn(ACharacter* Character, AController* Controller);
};

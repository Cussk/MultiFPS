// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MFPSGameStateBase.generated.h"


class AMFPSPlayerState;

UCLASS()
class MULTIFPS_API AMFPSGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AMFPSGameStateBase();
	
	bool HasFirstBloodBeenScored() const;
	void UpdateLeaders();
	AMFPSPlayerState* GetSoleLeader() const;
	bool IsTiedForLead(AMFPSPlayerState* PlayerState);
	
private:
	bool bHasFirstBloodBeenScored;
	
	UPROPERTY()
	TArray<TObjectPtr<AMFPSPlayerState>> Leaders;
};

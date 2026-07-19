// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/MFPSTypes.h"
#include "EliminationComponent.generated.h"


class AMFPSGameStateBase;
class AMFPSPlayerState;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MULTIFPS_API UEliminationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEliminationComponent();
	
	UFUNCTION()
	void OnRoundReported(FFiredRoundReport FiredRoundReport);
	
	UPROPERTY(EditDefaultsOnly, Category = "MFPS|Elimination")
	float SequentialKillInterval;
	
	UPROPERTY(EditDefaultsOnly, Category = "MFPS|Elimination")
	int32 KillsNeededForStreak;
	
private:
	AMFPSPlayerState* GetPlayerStateFromActor(AActor* Actor);
	void ProcessHitOrMiss(bool bHit, AMFPSPlayerState* AttackerPS);
	void ProcessKill(bool bHeadShot, AMFPSPlayerState* AttackerPS, AMFPSPlayerState* VictimPS);
	void ProcessHeadShot(bool bHeadShot, ESpecialKillTypes& OutKillType, AMFPSPlayerState* AttackerPS);
	void ProcessSequentialKills(ESpecialKillTypes& OutKillType, AMFPSPlayerState* AttackerPS);
	void ProcessStreaks(ESpecialKillTypes& OutKillType, AMFPSPlayerState* AttackerPS, AMFPSPlayerState* VictimPS);
	void ProcessFirstBlood(AMFPSGameStateBase* GameState, ESpecialKillTypes& OutKillType, AMFPSPlayerState* AttackerPS);
	void UpdateLeaderStatus(AMFPSGameStateBase* GameState, ESpecialKillTypes& OutKillType, AMFPSPlayerState* AttackerPS, AMFPSPlayerState* VictimPS);
	bool HasSpecialKillTypes(const ESpecialKillTypes& SpecialKillTypes) const;
	
	float LastKillTime;
	int32 SequentialKillCount;
	int32 StreakCount;
};

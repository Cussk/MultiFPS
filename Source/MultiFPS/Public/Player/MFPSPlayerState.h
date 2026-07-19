// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "GameFramework/PlayerState.h"
#include "Types/MFPSTypes.h"
#include "MFPSPlayerState.generated.h"

class UMFPSSpecialElim;
class USpecialElimData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, int32, NewScore);

UCLASS()
class MULTIFPS_API AMFPSPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AMFPSPlayerState();
	
	void AddScoredKill();
	void AddDefeat();
	void AddHit();
	void AddMiss();
	void AddHeadShotKill();
	void AddRevengeKill();
	void AddDethroneKill();
	void AddShowstopperKill();
	void GotFirstBlood();
	void SetWinner();
	void SetOnStreak(bool bIsOnStreak);
	void AddSequentialKill(int32 SequenceCount);
	void UpdateHighestKillStreak(int32 StreakCount);
	void SetLastKiller(APlayerState* NewLastKiller);
	
	bool IsOnStreak() const;
	bool IsWinner() const;
	bool IsFirstBlood() const;
	int32 GetScoreKills() const;
	int32 GetDefeats() const;
	int32 GetHits() const;
	int32 GetMisses() const;
	int32 GetHeadShotKills() const;
	int32 GetHighestKillStreak() const;
	int32 GetRevengeKills() const;
	APlayerState* GetLastKiller() const;
	
	UFUNCTION(Client, Reliable)
	void Client_LostTheLead();
	
	UFUNCTION(Client, Reliable)
	void Client_ScoredKill(int32 KillScore);
	
	UFUNCTION(Client, Reliable)
	void Client_SpecialKill(const ESpecialKillTypes& SpecialKillTypes, int32 SequentialKillCount, int32 StreakCount, int32 KillScore);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MFPS|SpecialElims")
	TObjectPtr<USpecialElimData> SpecialElimData;
	
	UPROPERTY(EditDefaultsOnly, Category = "MFPS|SpecialElims")
	TSubclassOf<UMFPSSpecialElim> SpecialElimWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "MFPS|SpecialElims")
	float KillDisplayTime;
	
	UPROPERTY(BlueprintAssignable)
	FOnScoreChanged OnScoreChanged;
	
private:
	TArray<ESpecialKillTypes> DecodeKillBitmask(ESpecialKillTypes KillTypeBitmask);
	void ProcessNextSpecialKill();
	void ShowSpecialKill(const FSpecialElimInfo& SpecialKillInfo);
	
	int32 ScoreKills;
	int32 Defeats;
	int32 Hits;
	int32 Misses;
	int32 HeadShotKills;
	int32 HighestKillStreak;
	int32 RevengeKills;
	int32 DethroneKills;
	int32 ShowstopperKills;
	bool bOnKillStreak;
	bool bFirstBlood;
	bool bWinner;
	TMap<int32, int32> SequentialKills;
	
	TWeakObjectPtr<APlayerState> LastKiller;
	
	TQueue<FSpecialElimInfo> SpecialKillQueue;
	bool bIsProcessingQueue;
};

// Copyright Kyle Cuss and Cuss Programming


#include "Player/MFPSPlayerState.h"

#include "TimerManager.h"
#include "Data/SpecialElimData.h"
#include "UI/Elims/MFPSSpecialElim.h"

AMFPSPlayerState::AMFPSPlayerState()
{
	SetNetUpdateFrequency(100.0f);
	
	ScoreKills = 0;
	Defeats = 0;
	Hits = 0;
	Misses = 0;
	HeadShotKills = 0;
	HighestKillStreak = 0;
	RevengeKills = 0;
	DethroneKills = 0;
	ShowstopperKills = 0;
	KillDisplayTime = 0.5f;
	bOnKillStreak = false;
	bFirstBlood = false;
	bWinner = false;
	bIsProcessingQueue = false;
}

void AMFPSPlayerState::AddScoredKill()
{
	++ScoreKills;
}

void AMFPSPlayerState::AddDefeat()
{
	++Defeats;
}

void AMFPSPlayerState::AddHit()
{
	++Hits;
}

void AMFPSPlayerState::AddMiss()
{
	++Misses;
}

void AMFPSPlayerState::AddHeadShotKill()
{
	++HeadShotKills;
}

void AMFPSPlayerState::AddRevengeKill()
{
	++RevengeKills;
}

void AMFPSPlayerState::AddDethroneKill()
{
	++DethroneKills;
}

void AMFPSPlayerState::AddShowstopperKill()
{
	++ShowstopperKills;
}

void AMFPSPlayerState::GotFirstBlood()
{
	bFirstBlood = true;
}

void AMFPSPlayerState::SetWinner()
{
	bWinner = true;
}

void AMFPSPlayerState::SetOnStreak(const bool bIsOnStreak)
{
	bOnKillStreak = bIsOnStreak;
}

void AMFPSPlayerState::AddSequentialKill(int32 SequenceCount)
{
	if (SequentialKills.Contains(SequenceCount))
	{
		SequentialKills[SequenceCount]++;
	}
	else
	{
		SequentialKills.Add(SequenceCount, 1);		
	}
	
	// Only count highest Sequence reached
	for (auto& Element : SequentialKills)
	{
		if (Element.Key < SequenceCount && Element.Value > 0)
		{
			Element.Value--;
		}
	}
}

void AMFPSPlayerState::UpdateHighestKillStreak(int32 StreakCount)
{
	if (StreakCount > HighestKillStreak)
	{
		HighestKillStreak = StreakCount;
	}
}

void AMFPSPlayerState::SetLastKiller(APlayerState* NewLastKiller)
{
	LastKiller = NewLastKiller;
}

bool AMFPSPlayerState::IsOnStreak() const
{
	return bOnKillStreak;
}

bool AMFPSPlayerState::IsWinner() const
{
	return bWinner;
}

bool AMFPSPlayerState::IsFirstBlood() const
{
	return bFirstBlood;
}

int32 AMFPSPlayerState::GetScoreKills() const
{
	return ScoreKills;
}

int32 AMFPSPlayerState::GetDefeats() const
{
	return Defeats;
}

int32 AMFPSPlayerState::GetHits() const
{
	return Hits;
}

int32 AMFPSPlayerState::GetMisses() const
{
	return Misses;
}

int32 AMFPSPlayerState::GetHeadShotKills() const
{
	return HeadShotKills;
}

int32 AMFPSPlayerState::GetHighestKillStreak() const
{
	return HighestKillStreak;
}

int32 AMFPSPlayerState::GetRevengeKills() const
{
	return RevengeKills;
}

APlayerState* AMFPSPlayerState::GetLastKiller() const
{
	return LastKiller.IsValid() ? LastKiller.Get() : nullptr;
}

TArray<ESpecialKillTypes> AMFPSPlayerState::DecodeKillBitmask(ESpecialKillTypes KillTypeBitmask)
{
	TArray<ESpecialKillTypes> ValidKillTypes;
	
	uint16 KillTypeBitmaskValue = static_cast<uint16>(KillTypeBitmask);
	
	for (uint16 i = 0; i < 16; ++i)
	{
		if (KillTypeBitmaskValue & (1 << i))
		{
			ESpecialKillTypes EnumValue = static_cast<ESpecialKillTypes>(1 << i);
			ValidKillTypes.Add(EnumValue);
		}
	}
	
	return ValidKillTypes;
}

void AMFPSPlayerState::ProcessNextSpecialKill()
{
	FSpecialElimInfo ElimMessageInfo;
	if (SpecialKillQueue.Dequeue(ElimMessageInfo))
	{
		bIsProcessingQueue = true;
		ShowSpecialKill(ElimMessageInfo);
		
		GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, this, &AMFPSPlayerState::ProcessNextSpecialKill, KillDisplayTime, false);
			bIsProcessingQueue = false;
		});
	}
	else
	{
		bIsProcessingQueue = false;
	}
}

void AMFPSPlayerState::ShowSpecialKill(const FSpecialElimInfo& SpecialKillInfo)
{
	FString ElimMessage = SpecialKillInfo.ElimMessage;
	if (SpecialKillInfo.SpecialKillType == ESpecialKillTypes::Sequential)
	{
		if (SpecialKillInfo.SequentialKillCount == 2) ElimMessage = FString("Double Kill!");
		else if (SpecialKillInfo.SequentialKillCount == 3) ElimMessage = FString("Triple Kill!");
		else if (SpecialKillInfo.SequentialKillCount == 4) ElimMessage = FString("Quadra Kill!");
		else if (SpecialKillInfo.SequentialKillCount > 4) ElimMessage = FString::Printf(TEXT("Rampage x%d!"), SpecialKillInfo.SequentialKillCount);
	}
	if (SpecialKillInfo.SpecialKillType == ESpecialKillTypes::Streak) ElimMessage = FString::Printf(TEXT("Streak x%d!"), SpecialKillInfo.StreakCount);
	
	if (IsValid(SpecialElimWidgetClass))
	{
		UMFPSSpecialElim* SpecialElimWidget = CreateWidget<UMFPSSpecialElim>(GetPlayerController(), SpecialElimWidgetClass);
		if (IsValid(SpecialElimWidget))
		{
			SpecialElimWidget->InitializeWidget(ElimMessage, SpecialKillInfo.ElimIcon);
			SpecialElimWidget->AddToViewport();
		}
	}
}

void AMFPSPlayerState::Client_ScoredKill_Implementation(int32 KillScore)
{
	
}

void AMFPSPlayerState::Client_SpecialKill_Implementation(const ESpecialKillTypes& SpecialKillTypes,
	int32 SequentialKillCount, int32 StreakCount, int32 KillScore)
{
	ensure(IsValid(SpecialElimData));
	
	TArray<ESpecialKillTypes> ValidKillTypes = DecodeKillBitmask(SpecialKillTypes);
	for (const ESpecialKillTypes& KillType : ValidKillTypes)
	{
		FSpecialElimInfo& ElimMessageInfo = SpecialElimData->SpecialKillInfo.FindChecked(KillType);
		
		if (KillType == ESpecialKillTypes::Sequential)
		{
			ElimMessageInfo.SequentialKillCount = SequentialKillCount;
		}
		if (KillType == ESpecialKillTypes::Streak)
		{
			ElimMessageInfo.StreakCount = StreakCount;
		}
		ElimMessageInfo.SpecialKillType = KillType;
		
		SpecialKillQueue.Enqueue(ElimMessageInfo);
	}
	
	if (!bIsProcessingQueue)
	{
		ProcessNextSpecialKill();
	}
}

void AMFPSPlayerState::Client_LostTheLead_Implementation()
{
	ensure(IsValid(SpecialElimData));
	
	FSpecialElimInfo& ElimMessageInfo = SpecialElimData->SpecialKillInfo.FindChecked(ESpecialKillTypes::LostTheLead);
	
	if (IsValid(SpecialElimWidgetClass))
	{
		UMFPSSpecialElim* SpecialElimWidget = CreateWidget<UMFPSSpecialElim>(GetPlayerController(), SpecialElimWidgetClass);
		if (IsValid(SpecialElimWidget))
		{
			SpecialElimWidget->InitializeWidget(ElimMessageInfo.ElimMessage, ElimMessageInfo.ElimIcon);
			SpecialElimWidget->AddToViewport();
		}
	}
}

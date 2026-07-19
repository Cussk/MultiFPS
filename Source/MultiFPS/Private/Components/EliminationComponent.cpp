// Copyright Kyle Cuss and Cuss Programming


#include "EliminationComponent.h"

#include "Debugging/SlateDebugging.h"
#include "Engine/World.h"
#include "Game/MFPSGameStateBase.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MFPSPlayerState.h"

UEliminationComponent::UEliminationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SequentialKillInterval = 2.0f;
	LastKillTime = 0.0f;
	SequentialKillCount = 0.0f;
	StreakCount = 0.0f;
	KillsNeededForStreak = 5;
}

void UEliminationComponent::OnRoundReported(FFiredRoundReport FiredRoundReport)
{
	AMFPSPlayerState* AttackerPS = GetPlayerStateFromActor(FiredRoundReport.Attacker);
	if (!IsValid(AttackerPS))
	{
		return;
	}
	
	ProcessHitOrMiss(FiredRoundReport.bHit, AttackerPS);
	
	if (!FiredRoundReport.bHit)
	{
		return;
	}
	
	AMFPSPlayerState* VictimPS = GetPlayerStateFromActor(FiredRoundReport.Victim);
	if (!IsValid(VictimPS))
	{
		return;
	}
	
	if (FiredRoundReport.bLethal)
	{
		ProcessKill(FiredRoundReport.bHeadShot, AttackerPS, VictimPS);
	}
}

void UEliminationComponent::ProcessHitOrMiss(bool bHit, AMFPSPlayerState* AttackerPS)
{
	if (bHit)
	{
		AttackerPS->AddHit();
	}
	else
	{
		AttackerPS->AddMiss();
	}
}

void UEliminationComponent::ProcessKill(bool bHeadShot, AMFPSPlayerState* AttackerPS, AMFPSPlayerState* VictimPS)
{
	AttackerPS->AddScoredKill();
	VictimPS->AddDefeat();
	
	ESpecialKillTypes SpecialKillTypes{};
	
	ProcessHeadShot(bHeadShot, SpecialKillTypes, AttackerPS);
	ProcessSequentialKills(SpecialKillTypes, AttackerPS);
	ProcessStreaks(SpecialKillTypes, AttackerPS, VictimPS);
	
	AMFPSGameStateBase* GameState = Cast<AMFPSGameStateBase>(UGameplayStatics::GetGameState(GetWorld()));
	if (IsValid(GameState))
	{
		UpdateLeaderStatus(GameState, SpecialKillTypes, AttackerPS, VictimPS);
		ProcessFirstBlood(GameState, SpecialKillTypes, AttackerPS);
	}
	
	if (HasSpecialKillTypes(SpecialKillTypes))
	{
		AttackerPS->Client_SpecialKill(SpecialKillTypes, SequentialKillCount, StreakCount, AttackerPS->GetScoreKills());
	}
	else
	{
		AttackerPS->Client_ScoredKill(AttackerPS->GetScoreKills());
	}
}

void UEliminationComponent::ProcessHeadShot(bool bHeadShot, ESpecialKillTypes& OutKillType, AMFPSPlayerState* AttackerPS)
{
	if (bHeadShot)
	{
		OutKillType |= ESpecialKillTypes::Headshot;
		AttackerPS->AddHeadShotKill();
	}
}

void UEliminationComponent::ProcessSequentialKills(ESpecialKillTypes& OutKillType, AMFPSPlayerState* AttackerPS)
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastKillTime <= SequentialKillInterval)
	{
		++SequentialKillCount;
	}
	else
	{
		SequentialKillCount = 1;
	}
	
	LastKillTime = CurrentTime;
	
	if (SequentialKillCount > 1)
	{
		OutKillType |= ESpecialKillTypes::Sequential;
		AttackerPS->AddSequentialKill(SequentialKillCount);
	}
}

void UEliminationComponent::ProcessStreaks(ESpecialKillTypes& OutKillType, AMFPSPlayerState* AttackerPS, AMFPSPlayerState* VictimPS)
{
	++StreakCount;
	if (StreakCount >= KillsNeededForStreak)
	{
		OutKillType |= ESpecialKillTypes::Streak;
		AttackerPS->SetOnStreak(true);
		AttackerPS->UpdateHighestKillStreak(StreakCount);
	}
	
	if (VictimPS->IsOnStreak())
	{
		OutKillType |= ESpecialKillTypes::Showstopper;
		AttackerPS->AddShowstopperKill();
		VictimPS->SetOnStreak(false);
	}
	
	if (AttackerPS->GetLastKiller() == VictimPS)
	{
		OutKillType |= ESpecialKillTypes::Revenge;
		AttackerPS->AddRevengeKill();
		AttackerPS->SetLastKiller(nullptr);
	}
	
	VictimPS->SetLastKiller(AttackerPS);
}

void UEliminationComponent::ProcessFirstBlood(AMFPSGameStateBase* GameState, ESpecialKillTypes& OutKillType, AMFPSPlayerState* AttackerPS)
{
	if (!GameState->HasFirstBloodBeenScored())
	{
		OutKillType |= ESpecialKillTypes::FirstBlood;
		AttackerPS->GotFirstBlood();
	}
}

void UEliminationComponent::UpdateLeaderStatus(AMFPSGameStateBase* GameState, ESpecialKillTypes& OutKillType,
	AMFPSPlayerState* AttackerPS, AMFPSPlayerState* VictimPS)
{
	AMFPSPlayerState* LastLeader = GameState->GetSoleLeader();
	const bool bAttackerWasTiedForLead = GameState->IsTiedForLead(AttackerPS);
	
	GameState->UpdateLeaders();
	
	if (!bAttackerWasTiedForLead && GameState->IsTiedForLead(AttackerPS))
	{
		OutKillType |= ESpecialKillTypes::TiedTheLeader;
	}
	
	if (IsValid(LastLeader) && LastLeader != GameState->GetSoleLeader())
	{
		LastLeader->Client_LostTheLead();
		
		if (VictimPS == LastLeader)
		{
			OutKillType |= ESpecialKillTypes::Dethrone;
			AttackerPS->AddDethroneKill();
		}
	}
	
	if (AttackerPS != LastLeader && AttackerPS == GameState->GetSoleLeader())
	{
		OutKillType |= ESpecialKillTypes::GainedTheLead;
	}
}

bool UEliminationComponent::HasSpecialKillTypes(const ESpecialKillTypes& SpecialKillTypes) const
{
	return static_cast<uint16>(SpecialKillTypes) != 0;
}

AMFPSPlayerState* UEliminationComponent::GetPlayerStateFromActor(AActor* Actor)
{
	APawn* Pawn = Cast<APawn>(Actor);
	if (IsValid(Pawn))
	{
		return  Pawn->GetPlayerState<AMFPSPlayerState>();
	}
	
	return nullptr;
}


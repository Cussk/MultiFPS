// Copyright Kyle Cuss and Cuss Programming


#include "Game/MFPSGameStateBase.h"

#include "Player/MFPSPlayerState.h"

AMFPSGameStateBase::AMFPSGameStateBase()
{
	bHasFirstBloodBeenScored = false;
}

bool AMFPSGameStateBase::HasFirstBloodBeenScored() const
{
	return bHasFirstBloodBeenScored;
}

void AMFPSGameStateBase::UpdateLeaders()
{
	TArray<APlayerState*> LocalSortedPlayers = PlayerArray;
	LocalSortedPlayers.Sort([](const APlayerState& A, const APlayerState& B)
	{
		const AMFPSPlayerState* APlayerState = Cast<AMFPSPlayerState>(&A);
		const AMFPSPlayerState* BPlayerState = Cast<AMFPSPlayerState>(&B);
		return APlayerState->GetScoreKills() > BPlayerState->GetScoreKills();
	});
	
	Leaders.Empty();
	
	if (LocalSortedPlayers.Num() > 0)
	{
		int32 HighScore = 0;
		for (APlayerState* Player : LocalSortedPlayers)
		{
			AMFPSPlayerState* MFPSPlayerState = Cast<AMFPSPlayerState>(Player);
			if (!IsValid(MFPSPlayerState)) continue;
			
			const int32 PlayerScore = MFPSPlayerState->GetScoreKills();
			
			if (Leaders.Num() == 0)
			{
				HighScore = PlayerScore;
				Leaders.Add(MFPSPlayerState);
			}
			else if (PlayerScore == HighScore)
			{
				Leaders.Add(MFPSPlayerState);
			}
			else
			{
				break;
			}
		}
	}
	
	bHasFirstBloodBeenScored = true;
}

AMFPSPlayerState* AMFPSGameStateBase::GetSoleLeader() const
{
	if (Leaders.Num() == 1)
	{
		return Leaders[0];
	}
	
	return nullptr;
}

bool AMFPSGameStateBase::IsTiedForLead(AMFPSPlayerState* PlayerState)
{
	return Leaders.Contains(PlayerState) && Leaders.Num() > 1;
}

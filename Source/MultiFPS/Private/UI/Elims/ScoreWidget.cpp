// Copyright Kyle Cuss and Cuss Programming


#include "UI/Elims/ScoreWidget.h"

#include "Components/TextBlock.h"
#include "Player/MFPSPlayerController.h"
#include "Player/MFPSPlayerState.h"

void UScoreWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	AMFPSPlayerState* PS = GetPlayerState();
	if (IsValid(PS))
	{
		PS->OnScoreChanged.AddDynamic(this, &UScoreWidget::OnScoreChanged);
	}
	else
	{
		AMFPSPlayerController* PC = Cast<AMFPSPlayerController>(GetOwningPlayer());
		if (IsValid(PC))
		{
			PC->OnPlayerStateReplicated.AddUniqueDynamic(this, &UScoreWidget::OnPlayerStateReplicated);
		}
	}
}

AMFPSPlayerState* UScoreWidget::GetPlayerState() const
{
	APlayerController* PC = GetOwningPlayer();
	if (IsValid(PC))
	{
		return PC->GetPlayerState<AMFPSPlayerState>();
	}
	
	return nullptr;
}

void UScoreWidget::OnScoreChanged(int32 NewScore)
{
	if (IsValid(ScoreText))
	{
		ScoreText->SetText(FText::AsNumber(NewScore));
	}
}

void UScoreWidget::OnPlayerStateReplicated()
{
	AMFPSPlayerState* PS = GetPlayerState();
	if (IsValid(PS))
	{
		PS->OnScoreChanged.AddDynamic(this, &UScoreWidget::OnScoreChanged);
		OnScoreChanged(PS->GetScoreKills());
	}
	
	AMFPSPlayerController* PC = Cast<AMFPSPlayerController>(GetOwningPlayer());
	if (IsValid(PC))
	{
		PC->OnPlayerStateReplicated.RemoveDynamic(this, &UScoreWidget::OnPlayerStateReplicated);
	}
}

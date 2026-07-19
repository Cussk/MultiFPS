// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreWidget.generated.h"

class UTextBlock;
class AMFPSPlayerState;

UCLASS()
class MULTIFPS_API UScoreWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnInitialized() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreText;
	
private:
	AMFPSPlayerState* GetPlayerState() const;
	
	UFUNCTION()
	void OnScoreChanged(int32 NewScore);
	
	UFUNCTION()
	void OnPlayerStateReplicated();
};

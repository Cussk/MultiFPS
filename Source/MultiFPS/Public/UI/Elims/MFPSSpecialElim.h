// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MFPSSpecialElim.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class MULTIFPS_API UMFPSSpecialElim : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void CenterWidget(UUserWidget* Widget, float VerticalRatio = 0.0f);
	
	void InitializeWidget(const FString& ElimMessage, UTexture2D* ElimTexture);
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ElimMessageText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ElimTypeImage;
};

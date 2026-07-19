// Copyright Kyle Cuss and Cuss Programming


#include "UI/Elims/MFPSSpecialElim.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UMFPSSpecialElim::CenterWidget(UUserWidget* Widget, float VerticalRatio)
{
	if (!IsValid(Widget))
	{
		return;
	}
	
	FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(Widget);
	const float VerticalFraction = VerticalRatio == 0.0f ? 1.0f : VerticalRatio * 2.0f;
	FVector2D CenterPosition(ViewportSize.X * 0.5f, ViewportSize.Y * VerticalFraction * 0.5f);
	Widget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
	Widget->SetPositionInViewport(CenterPosition, true);
}

void UMFPSSpecialElim::InitializeWidget(const FString& ElimMessage, UTexture2D* ElimTexture)
{
	if (IsValid(ElimMessageText))
	{
		ElimMessageText->SetText(FText::FromString(ElimMessage));
	}
	
	if (IsValid(ElimTypeImage))
	{
		ElimTypeImage->SetBrushFromTexture(ElimTexture);
	}
}

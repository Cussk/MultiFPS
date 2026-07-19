// Copyright Kyle Cuss and Cuss Programming

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/MFPSTypes.h"
#include "SpecialElimData.generated.h"

UCLASS()
class MULTIFPS_API USpecialElimData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MFPS|SpecialElims")
	TMap<ESpecialKillTypes, FSpecialElimInfo> SpecialKillInfo;
};

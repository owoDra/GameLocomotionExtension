// Copyright (C) 2023 owoDra

#pragma once

#include "InAirState.generated.h"

USTRUCT(BlueprintType)
struct GCLADDON_API FInAirState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ForceUnits = "cm/s"))
	float VerticalVelocity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	bool bJumped = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ForceUnits = "x"))
	float JumpPlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float GroundPredictionAmount = 1.0f;
};

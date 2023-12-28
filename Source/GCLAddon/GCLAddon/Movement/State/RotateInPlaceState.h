﻿// Copyright (C) 2023 owoDra

#pragma once

#include "RotateInPlaceState.generated.h"

USTRUCT(BlueprintType)
struct GCLADDON_API FRotateInPlaceState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	bool bRotatingLeft = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	bool bRotatingRight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ForceUnits = "x"))
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	float FootLockBlockAmount = 0.0f;
};

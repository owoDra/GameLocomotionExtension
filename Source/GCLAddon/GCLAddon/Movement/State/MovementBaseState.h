// Copyright (C) 2023 owoDra

#pragma once

#include "MovementBaseState.generated.h"

USTRUCT(BlueprintType)
struct GCLADDON_API FMovementBaseState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	TObjectPtr<UPrimitiveComponent> Primitive = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	bool bBaseChanged = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	bool bHasRelativeLocation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	bool bHasRelativeRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FVector Location = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FQuat Rotation = FQuat(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FRotator DeltaRotation = FRotator(ForceInit);
};

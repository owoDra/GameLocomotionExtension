// Copyright (C) 2024 owoDra

#pragma once

#include "MovementBaseState.generated.h"


USTRUCT(BlueprintType)
struct GLEXT_API FMovementBaseState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UPrimitiveComponent> Primitive{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBaseChanged{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasRelativeLocation{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasRelativeRotation{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location{ ForceInit };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FQuat Rotation{ ForceInit };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator DeltaRotation{ ForceInit };

};

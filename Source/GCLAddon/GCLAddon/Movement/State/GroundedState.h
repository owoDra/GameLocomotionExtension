// Copyright (C) 2023 owoDra

#pragma once

#include "MovementDirection.h"
#include "GroundedState.generated.h"

UENUM(BlueprintType)
enum class EHipsDirection : uint8
{
	Forward,
	Backward,
	LeftForward,
	LeftBackward,
	RightForward,
	RightBackward,
};

USTRUCT(BlueprintType)
struct GCLADDON_API FVelocityBlendState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	bool bReinitializationRequired = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float ForwardAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float BackwardAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float LeftAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float RightAmount = 0.0f;
};

USTRUCT(BlueprintType)
struct GCLADDON_API FRotationYawOffsetsState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float ForwardAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float BackwardAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float LeftAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float RightAngle = 0.0f;
};

USTRUCT(BlueprintType)
struct GCLADDON_API FGroundedState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	EHipsDirection HipsDirection = EHipsDirection::Forward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = -1, ClampMax = 1))
	float HipsDirectionLockAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	bool bPivotActivationRequested = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	bool bPivotActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FMovementDirectionCache MovementDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FVelocityBlendState VelocityBlend;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FRotationYawOffsetsState RotationYawOffsets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ForceUnits = "s"))
	float SprintTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = -1, ClampMax = 1))
	float SprintAccelerationAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float SprintBlockAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float WalkRunBlendAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ClampMax = 1))
	float StrideBlendAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ForceUnits = "x"))
	float StandingPlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ForceUnits = "x"))
	float CrouchingPlayRate = 1.0f;
};

﻿// Copyright (C) 2024 owoDra

#pragma once

#include "AnimationLocomotionState.generated.h"


USTRUCT(BlueprintType)
struct GLEXT_API FAnimationLocomotionState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasInput{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float InputYawAngle{ 0.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float Speed{ 0.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity{ ForceInit };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float VelocityYawAngle{ 0.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Acceleration{ ForceInit };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0))
	float MaxAcceleration{ 0.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0))
	float MaxBrakingDeceleration{ 0.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WalkableFloorZ{ 0.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMoving{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMovingSmooth{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float TargetYawAngle{ 0.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location{ ForceInit };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation{ ForceInit };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FQuat RotationQuaternion{ ForceInit };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ForceUnits = "deg/s"))
	float YawSpeed{ 0.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ForceUnits = "x"))
	float Scale{ 1.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "cm"))
	float CapsuleRadius{ 0.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "cm"))
	float CapsuleHalfHeight{ 0.0f };

};

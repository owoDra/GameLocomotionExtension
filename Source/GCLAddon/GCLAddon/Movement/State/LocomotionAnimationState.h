// Copyright (C) 2023 owoDra

#pragma once

#include "LocomotionAnimationState.generated.h"

USTRUCT(BlueprintType)
struct GCLADDON_API FLocomotionAnimationState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	bool bHasInput = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float InputYawAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FVector Velocity = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float VelocityYawAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FVector Acceleration = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0))
	float MaxAcceleration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0))
	float MaxBrakingDeceleration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	float WalkableFloorZ = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	bool bMoving = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	bool bMovingSmooth = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float TargetYawAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FVector Location = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FRotator Rotation = FRotator(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FQuat RotationQuaternion = FQuat(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ForceUnits = "deg/s"))
	float YawSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ForceUnits = "x"))
	float Scale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float CapsuleRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float CapsuleHalfHeight = 0.0f;
};

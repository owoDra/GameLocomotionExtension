// Copyright (C) 2023 owoDra

#pragma once

#include "LocomotionState.generated.h"

USTRUCT(BlueprintType)
struct GCLADDON_API FLocomotionState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	bool bHasInput = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float InputYawAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	bool bHasSpeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FVector Velocity = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FVector PreviousVelocity = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float VelocityYawAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FVector Acceleration = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	bool bMoving = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	bool bRotationTowardsLastInputDirectionBlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float TargetYawAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float ViewRelativeTargetYawAngle = 0.0f;

	// Used for extra smooth actor rotation, in other cases equal to the regular target yaw angle.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float SmoothTargetYawAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FVector Location = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FRotator Rotation = FRotator(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "")
	FQuat RotationQuaternion = FQuat(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float PreviousYawAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "", Meta = (ForceUnits = "deg/s"))
	float YawSpeed = 0.0f;
};

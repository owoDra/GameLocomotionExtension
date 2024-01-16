// Copyright (C) 2024 owoDra

#pragma once

#include "ViewState.generated.h"


USTRUCT(BlueprintType)
struct GLEXT_API FViewNetworkSmoothingState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled{ false };

	// Used to track the time of the last server move.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "s"))
	float ServerTime{ 0.0f };

	// Used to track client time as we try to match the server.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "s"))
	float ClientTime{ 0.0f };

	// Used for remembering how much time passed between server corrections.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "s"))
	float Duration{ 0.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator InitialRotation{ ForceInit };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation{ ForceInit };

};


USTRUCT(BlueprintType)
struct GLEXT_API FViewState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FViewNetworkSmoothingState NetworkSmoothing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation{ ForceInit };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float YawSpeed{ 0.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float PreviousYawAngle{ 0.0f };

};

// Copyright (C) 2023 owoDra

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "LocomotionGeneralNameStatics.generated.h"


/**
 * Class that stores constants for any name related to the character
 */
UCLASS(Meta = (BlueprintThreadSafe))
class GCLADDON_API ULocomotionGeneralNameStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/////////////////////////////////////
	// General Bone 
public:
	UFUNCTION(BlueprintPure, Category = "General|Bone", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& RootBoneName()
	{
		static const FName Name = FName(TEXTVIEW("root"));
		return Name;
	}


	/////////////////////////////////////
	// General Animation Curves
public:
	UFUNCTION(BlueprintPure, Category = "General|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& ViewBlockCurveName()
	{
		static const FName Name = FName(TEXTVIEW("ViewBlock"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "General|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& AllowAimingCurveName()
	{
		static const FName Name = FName(TEXTVIEW("AllowAiming"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "General|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& RotationYawSpeedCurveName()
	{
		static const FName Name = FName(TEXTVIEW("RotationYawSpeed"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "General|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& RotationYawOffsetCurveName()
	{
		static const FName Name = FName(TEXTVIEW("RotationYawOffset"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "General|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& AllowTransitionsCurveName()
	{
		static const FName Name = FName(TEXTVIEW("AllowTransitions"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "General|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& GroundPredictionBlockCurveName()
	{
		static const FName Name = FName(TEXTVIEW("GroundPredictionBlock"));
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "General|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& FootstepSoundBlockCurveName()
	{
		static const FName Name = FName(TEXTVIEW("FootstepSoundBlock"));
		return Name;
	}

};
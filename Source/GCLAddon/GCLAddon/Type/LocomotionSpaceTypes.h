// Copyright (C) 2023 owoDra

#pragma once

#include "LocomotionSpaceTypes.generated.h"


/**
 * Space in which the character moves
 * 
 * Tips:
 *	No matter what the movement mode is, 
 *	there are only three types of space to move: "in the air," "on the ground," and "in the water," 
 *	therefore, the enumeration is used to determine the movement space.
 */
UENUM(BlueprintType)
enum class ELocomotionSpace : uint8
{
	// When movement mode is "None"
	None,

	// Moving in the air
	InAir,

	// Moving on the ground
	OnGround,

	// Moving in the water
	InWater
};

// Copyright (C) 2023 owoDra

#include "CharacterMovementData.h"

#include "GameplayTag/GCLATags_Status.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterMovementData)


UCharacterMovementData::UCharacterMovementData()
{
	LocomotionModes =
	{
		{ TAG_Status_LocomotionMode_OnGround, FCharacterLocomotionModeConfigs() },
		{ TAG_Status_LocomotionMode_InAir	, FCharacterLocomotionModeConfigs() },
		{ TAG_Status_LocomotionMode_InWater	, FCharacterLocomotionModeConfigs() }
	};
}

const FCharacterLocomotionModeConfigs& UCharacterMovementData::GetAllowedLocomotionMode(const FGameplayTag& LomotionMode) const
{
	// Search LocomotionModeConfigs from DesiredLocomotionMode

	auto* Configs{ LocomotionModes.Find(LomotionMode) };

	check(Configs);

	return *Configs;
}

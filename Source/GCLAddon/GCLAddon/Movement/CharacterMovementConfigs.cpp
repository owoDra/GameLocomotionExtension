// Copyright (C) 2023 owoDra

#include "CharacterMovementConfigs.h"

#include "GCLACharacterMovementComponent.h"
#include "Movement/CharacterMovementCondition.h"
#include "GameplayTag/GCLATags_Status.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterMovementConfigs)


/////////////////////////////////////////
// FCharacterStanceConfigs

FCharacterStanceConfigs::FCharacterStanceConfigs()
{
	DefaultGait = TAG_Status_Gait_Walking;

	Gaits =
	{ 
		{
			TAG_Status_Gait_Walking,
			FCharacterGaitConfigs(
				175.0,
				2500.0,
				2000.0,
				10.0,
				420.0,
				0.25,
				8.0,
				nullptr
			)
		},
		{
			TAG_Status_Gait_Running,
			FCharacterGaitConfigs(
				375.0,
				2500.0,
				2000.0,
				10.0,
				420.0,
				0.25,
				8.0,
				nullptr
			)
		},
		{
			TAG_Status_Gait_Sprinting,
			FCharacterGaitConfigs(
				650.0,
				2500.0,
				2000.0,
				10.0,
				420.0,
				0.25,
				8.0,
				nullptr
			)
		},
	};
}

const FCharacterGaitConfigs& FCharacterStanceConfigs::GetAllowedGait(const UGCLACharacterMovementComponent* CMC, const FGameplayTag& DesiredGait, FGameplayTag& OutTag) const
{
	// Search GaitConfigs from DesiredGait

	const auto* Configs{ Gaits.Find(DesiredGait) };
	if (Configs)
	{
		// If Condition is not set, return as is

		if (!Configs->Condition)
		{
			OutTag = DesiredGait;
			return *Configs;
		}

		//  Returns GaitConfigs based on SuggestStateTag of Condition if transition is not possible.

		if (Configs->Condition->CanEnter(CMC))
		{
			OutTag = DesiredGait;
			return *Configs;
		}

		// Returns GaitConfigs based on SuggestStateTag of Condition if transition is not possible.

		const auto& SuggestTag{ Configs->Condition->SuggestStateTag };

		if (SuggestTag.IsValid())
		{
			return GetAllowedGait(CMC, SuggestTag, OutTag);
		}
	}

	// Returns GaitConfigs based on DefaultGait if no transitive GaitConfigs are found

	Configs = Gaits.Find(DefaultGait);
	check(Configs);

	OutTag = DefaultGait;
	return *Configs;
}


/////////////////////////////////////////
// FCharacterRotationModeConfigs

FCharacterRotationModeConfigs::FCharacterRotationModeConfigs()
{
	DefaultStance = TAG_Status_Stance_Standing;

	Stances =
	{
		{ TAG_Status_Stance_Standing, FCharacterStanceConfigs() },
		{ TAG_Status_Stance_Crouching, FCharacterStanceConfigs() }
	};

	Condition = nullptr;
}

const FCharacterStanceConfigs& FCharacterRotationModeConfigs::GetAllowedStance(const FGameplayTag& DesiredStance, FGameplayTag& OutTag) const
{
	// Search StanceConfigs from DesiredStance

	const auto* Configs{ Stances.Find(DesiredStance) };
	if (Configs)
	{
		OutTag = DesiredStance;
		return *Configs;
	}

	// Returns StanceConfigs based on DefaultStance if no transitionable StanceConfigs are found

	Configs = Stances.Find(DefaultStance);
	check(Configs);

	OutTag = DefaultStance;
	return *Configs;
}


/////////////////////////////////////////
// FCharacterLocomotionModeConfigs

FCharacterLocomotionModeConfigs::FCharacterLocomotionModeConfigs()
{
	DefaultRotationMode = TAG_Status_RotationMode_ViewDirection;

	RotationModes =
	{
		{ TAG_Status_RotationMode_VelocityDirection, FCharacterRotationModeConfigs() },
		{ TAG_Status_RotationMode_ViewDirection, FCharacterRotationModeConfigs() },
		{ TAG_Status_RotationMode_Aiming, FCharacterRotationModeConfigs() }
	};
}

const FCharacterRotationModeConfigs& FCharacterLocomotionModeConfigs::GetAllowedRotationMode(const UGCLACharacterMovementComponent* CMC, const FGameplayTag& DesiredRotationMode, FGameplayTag& OutTag) const
{
	// Search for RotationModeConfigs from DesiredRotationMode

	const auto* Configs{ RotationModes.Find(DesiredRotationMode) };
	if (Configs)
	{
		// Condition が設定されていなければそのまま返す

		if (!Configs->Condition)
		{
			OutTag = DesiredRotationMode;
			return *Configs;
		}

		// If Condition is not set, return as is

		if (Configs->Condition->CanEnter(CMC))
		{
			OutTag = DesiredRotationMode;
			return *Configs;
		}

		// If transition is not possible, RotationModeConfigs is returned based on the SuggestStateTag of the Condition.

		const auto& SuggestTag{ Configs->Condition->SuggestStateTag };

		if (SuggestTag.IsValid())
		{
			return GetAllowedRotationMode(CMC, SuggestTag, OutTag);
		}
	}

	// Returns RotationModeConfigs based on DefaultRotationMode if no transitionable RotationModeConfigs were found

	Configs = RotationModes.Find(DefaultRotationMode);
	check(Configs);

	OutTag = DefaultRotationMode;
	return *Configs;
}

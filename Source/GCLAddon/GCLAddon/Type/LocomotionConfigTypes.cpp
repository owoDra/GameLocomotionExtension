// Copyright (C) 2023 owoDra

#include "LocomotionConfigTypes.h"

#include "Condition/LocomotionCondition.h"
#include "LocomotionComponent.h"
#include "GameplayTag/GCLATags_Status.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LocomotionConfigTypes)


namespace LocomotionConfigHelper
{
	template<typename ReturnType>
	static const ReturnType& GetAllowedConfig(const FGameplayTag& DefaultState, const TMap<FGameplayTag, ReturnType>& ConfigMap,
		const ULocomotionComponent* LC, const FGameplayTag& DesiredState, FGameplayTag& OutState)
	{
		// Search Configs from ConfigMap

		const auto* Configs{ ConfigMap.Find(DesiredState) };
		if (Configs)
		{
			// If Condition is not set, return as is

			const auto& Condition{ Configs->Condition };

			if (!Condition)
			{
				OutState = DesiredState;
				return *Configs;
			}

			//  Checks for Condition

			if (Condition->CanEnter(LC))
			{
				OutState = DesiredState;
				return *Configs;
			}

			// Returns Configs based on SuggestStateTag of Condition if transition is not possible.

			const auto& SuggestTag{ Condition->SuggestStateTag };

			if (SuggestTag.IsValid())
			{
				return LocomotionConfigHelper::GetAllowedConfig<ReturnType>(DefaultState, ConfigMap, LC, SuggestTag, OutState);
			}
		}

		// Returns Configs based on DefaultState if transitive Configs are not found

		Configs = ConfigMap.Find(DefaultState);
		check(Configs);

		OutState = DefaultState;
		return *Configs;
	}
}


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

	Condition = nullptr;
}

const FCharacterGaitConfigs& FCharacterStanceConfigs::GetAllowedGait(const ULocomotionComponent* LC, const FGameplayTag& DesiredGait, FGameplayTag& OutState) const
{
	return LocomotionConfigHelper::GetAllowedConfig<FCharacterGaitConfigs>(DefaultGait, Gaits, LC, DesiredGait, OutState);
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

const FCharacterStanceConfigs& FCharacterRotationModeConfigs::GetAllowedStance(const ULocomotionComponent* LC, const FGameplayTag& DesiredStance, FGameplayTag& OutState) const
{
	return LocomotionConfigHelper::GetAllowedConfig<FCharacterStanceConfigs>(DefaultStance, Stances, LC, DesiredStance, OutState);
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

const FCharacterRotationModeConfigs& FCharacterLocomotionModeConfigs::GetAllowedRotationMode(const ULocomotionComponent* LC, const FGameplayTag& DesiredRotationMode, FGameplayTag& OutState) const
{
	return LocomotionConfigHelper::GetAllowedConfig<FCharacterRotationModeConfigs>(DefaultRotationMode, RotationModes, LC, DesiredRotationMode, OutState);
}

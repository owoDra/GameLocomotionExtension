// Copyright (C) 2023 owoDra

#include "LocomotionData.h"

#include "Condition/LocomotionCondition.h"
#include "GameplayTag/GCLATags_Status.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LocomotionData)


ULocomotionData::ULocomotionData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MovementModeToLocomotionMode =
	{
		{ EMovementMode::MOVE_Walking	, TAG_Status_LocomotionMode_OnGround },
		{ EMovementMode::MOVE_NavWalking, TAG_Status_LocomotionMode_OnGround },
		{ EMovementMode::MOVE_Falling	, TAG_Status_LocomotionMode_InAir },
		{ EMovementMode::MOVE_Flying	, TAG_Status_LocomotionMode_InAir },
		{ EMovementMode::MOVE_Swimming	, TAG_Status_LocomotionMode_InWater },
	};

	LocomotionModes =
	{
		{ TAG_Status_LocomotionMode_OnGround, FCharacterLocomotionModeConfigs() },
		{ TAG_Status_LocomotionMode_InAir	, FCharacterLocomotionModeConfigs() },
		{ TAG_Status_LocomotionMode_InWater	, FCharacterLocomotionModeConfigs() }
	};
}


const FCharacterLocomotionModeConfigs& ULocomotionData::GetLocomotionModeConfig(const FGameplayTag& LomotionMode) const
{
	auto* Configs{ LocomotionModes.Find(LomotionMode) };

	check(Configs);

	return *Configs;
}

FGameplayTag ULocomotionData::Convert_MovementModeToLocomotionMode(const EMovementMode& MovementMode, const uint8& CustomMovementMode) const
{
	if (MovementMode == MOVE_Custom)
	{
		return CustomMovementModeToLocomotionMode.FindRef(CustomMovementMode);
	}
	else
	{
		return MovementModeToLocomotionMode.FindRef(MovementMode);
	}

	return FGameplayTag();
}

void ULocomotionData::Convert_LocomotionModeToMovementMode(const FGameplayTag& LocomotionMode, EMovementMode& MovementMode, uint8& CustomMovementMode) const
{
	MovementMode = MOVE_None;
	CustomMovementMode = 0;

	if (const auto* FoundMovementMode{ MovementModeToLocomotionMode.FindKey(LocomotionMode) })
	{
		MovementMode = *FoundMovementMode;
	}
	else if(const auto* FoundCustomMovementMode{ CustomMovementModeToLocomotionMode.FindKey(LocomotionMode) })
	{
		MovementMode = MOVE_Custom;
		CustomMovementMode = *FoundCustomMovementMode;
	}
}

bool ULocomotionData::CanChangeMovementModeTo(const ULocomotionComponent* LC, const EMovementMode& MovementMode, const uint8& CustomMovementMode) const
{
	// false if no matching LocomotionMode is found

	const auto LocomotionModeTag{ Convert_MovementModeToLocomotionMode(MovementMode, CustomMovementMode) };

	if (!LocomotionModeTag.IsValid())
	{
		return false;
	}

	// true if Condition is not set

	const auto& Configs{ GetLocomotionModeConfig(LocomotionModeTag) };
	const auto& Condition{ Configs.Condition };

	if (!Condition)
	{
		return true;
	}

	// Return the result of Condition

	return Condition->CanEnter(LC);
}

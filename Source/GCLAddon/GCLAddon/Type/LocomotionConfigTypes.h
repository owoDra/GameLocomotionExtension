// Copyright (C) 2023 owoDra

#pragma once

#include "LocomotionSpaceTypes.h"

#include "GameplayTagContainer.h"

#include "LocomotionConfigTypes.generated.h"

class ULocomotionCondition;
class ULocomotionComponent;


/**
 * Data for each Gait config of the character
 */
USTRUCT(BlueprintType)
struct FCharacterGaitConfigs
{
	GENERATED_BODY()
public:
	FCharacterGaitConfigs() {}

	FCharacterGaitConfigs(float InMaxSpeed, float InMaxAcceleration, float InBrakingDeceleration, float InGroundFriction, float InJumpZPower, float InAirControl, float InRotationInterpSpeed, const TObjectPtr<ULocomotionCondition>& InCondition)
		: MaxSpeed(InMaxSpeed)
		, MaxAcceleration(InMaxAcceleration)
		, BrakingDeceleration(InBrakingDeceleration)
		, GroundFriction(InGroundFriction)
		, JumpZPower(InJumpZPower)
		, AirControl(InAirControl)
		, RotationInterpSpeed(InRotationInterpSpeed)
		, Condition(InCondition)
	{}

public:
	//
	// Maximum movement speed while this Gait is being applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float MaxSpeed{ 0.0f };

	//
	// Maximum acceleration during which this Gait is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ForceUnits = "cm/s"))
	float MaxAcceleration{ 0.0f };

	//
	// Deceleration during which this Gait is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ForceUnits = "cm/s"))
	float BrakingDeceleration{ 0.0f };

	//
	// Friction factor during this Gait is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GroundFriction{ 1.0f };

	//
	// Jump force during this Gait is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float JumpZPower{ 400.0f };

	//
	// Air control during this Gait is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AirControl{ 1.0f };

	//
	// Rotation interp speed during this Gait is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationInterpSpeed{ 16.0f };

	//
	// Conditions to determine if a transition is possible
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TObjectPtr<const ULocomotionCondition> Condition{ nullptr };

};


/**
 * Data for each stance config of the character.
 */
USTRUCT(BlueprintType)
struct FCharacterStanceConfigs
{
	GENERATED_BODY()
public:
	FCharacterStanceConfigs();

public:
	//
	// Gait to be transitioned when an invalid Gait is set.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Categories = "Status.Gait"))
	FGameplayTag DefaultGait;

	//
	// Gait allowed during this Stance is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Categories = "Status.Gait", ForceInlineRow))
	TMap<FGameplayTag, FCharacterGaitConfigs> Gaits;

	//
	// Conditions to determine if a transition is possible
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TObjectPtr<const ULocomotionCondition> Condition{ nullptr };

public:
	/**
	 * Get a GaitTag that can be transitioned
	 */
	const FCharacterGaitConfigs& GetAllowedGait(const ULocomotionComponent* LC, const FGameplayTag& DesiredGait, FGameplayTag& OutState) const;

};


/**
 * Data for each RotationMode config of character.
 */
USTRUCT(BlueprintType)
struct FCharacterRotationModeConfigs
{
	GENERATED_BODY()
public:
	FCharacterRotationModeConfigs();

public:
	//
	// Gait to be transitioned when an invalid stance is set.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Categories = "Status.Stance"))
	FGameplayTag DefaultStance;

	//
	// Stance allowed during this RotationMode is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Categories = "Status.Stance", ForceInlineRow))
	TMap<FGameplayTag, FCharacterStanceConfigs> Stances;

	//
	// Conditions to determine if a transition is possible
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TObjectPtr<const ULocomotionCondition> Condition;

public:
	/**
	 * Get a StanceTag that can be transposed.
	 */
	const FCharacterStanceConfigs& GetAllowedStance(const ULocomotionComponent* LC, const FGameplayTag& DesiredStance, FGameplayTag& OutState) const;

};


/**
 * Data for each LocomotionMode config of character
 */
USTRUCT(BlueprintType)
struct FCharacterLocomotionModeConfigs
{
	GENERATED_BODY()
public:
	FCharacterLocomotionModeConfigs();

public:
	//
	// Which LocomotionSpace this LocomotionMode moves
	// 
	// Note:
	//	This value must not be "None"
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELocomotionSpace LocomotionSpace{ ELocomotionSpace::None };

	//
	// Gait to be transitioned when an invalid rotation mode is set.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Categories = "Status.RotationMode"))
	FGameplayTag DefaultRotationMode;

	//
	// RotationMode allowed during this LocomotionMode is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Categories = "Status.RotationMode", ForceInlineRow))
	TMap<FGameplayTag, FCharacterRotationModeConfigs> RotationModes;

	//
	// Conditions to determine if a transition is possible
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TObjectPtr<const ULocomotionCondition> Condition{ nullptr };

public:
	/**
	 * Get a RotationModeTag that can be transposed.
	 */
	const FCharacterRotationModeConfigs& GetAllowedRotationMode(const ULocomotionComponent* LC, const FGameplayTag& DesiredRotationMode, FGameplayTag& OutState) const;

};

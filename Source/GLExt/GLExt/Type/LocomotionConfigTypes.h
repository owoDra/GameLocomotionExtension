// Copyright (C) 2024 owoDra

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
	FCharacterGaitConfigs();

	FCharacterGaitConfigs(float InMaxSpeed, float InMaxAcceleration, float InBrakingDeceleration, float InGroundFriction, float InJumpZPower, float InAirControl, float InRotationInterpSpeed, const TObjectPtr<ULocomotionCondition>& InCondition);

public:
	//
	// Conditions to determine if a transition is possible
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TObjectPtr<const ULocomotionCondition> Condition{ nullptr };

	//
	// Maximum movement speed while this Gait is being applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float MaxSpeed;

	//
	// Maximum acceleration during which this Gait is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxAcceleration;

	//
	// Deceleration during which this Gait is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingDeceleration;

	//
	// Friction factor during this Gait is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GroundFriction;

	//
	// Jump force during this Gait is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float JumpZPower;

	//
	// Air control during this Gait is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AirControl;

	//
	// Rotation interp speed during this Gait is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationInterpSpeed;

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
	// Conditions to determine if a transition is possible
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TObjectPtr<const ULocomotionCondition> Condition{ nullptr };

	//
	// Gait to be transitioned when an invalid Gait is set.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Categories = "Status.Gait"))
	FGameplayTag DefaultGait;

	//
	// Gait allowed during this Stance is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ForceInlineRow, Categories = "Status.Gait", ForceInlineRow))
	TMap<FGameplayTag, FCharacterGaitConfigs> Gaits;


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
	// Conditions to determine if a transition is possible
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TObjectPtr<const ULocomotionCondition> Condition;

	//
	// Gait to be transitioned when an invalid stance is set.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Categories = "Status.Stance"))
	FGameplayTag DefaultStance;

	//
	// Stance allowed during this RotationMode is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ForceInlineRow, Categories = "Status.Stance"))
	TMap<FGameplayTag, FCharacterStanceConfigs> Stances;

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
	// Conditions to determine if a transition is possible
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TObjectPtr<const ULocomotionCondition> Condition{ nullptr };

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ForceInlineRow, Categories = "Status.RotationMode"))
	TMap<FGameplayTag, FCharacterRotationModeConfigs> RotationModes;


public:
	/**
	 * Get a RotationModeTag that can be transposed.
	 */
	const FCharacterRotationModeConfigs& GetAllowedRotationMode(const ULocomotionComponent* LC, const FGameplayTag& DesiredRotationMode, FGameplayTag& OutState) const;

};

// Copyright (C) 2023 owoDra

#pragma once

#include "CharacterMovementConfigs.generated.h"

class UGCLACharacterMovementComponent;
class UCharacterMovementCondition;
struct FGameplayTag;


/**
 * Data for each Gait config of the character
 */
USTRUCT(BlueprintType)
struct FCharacterGaitConfigs
{
	GENERATED_BODY()
public:
	FCharacterGaitConfigs() {}

	FCharacterGaitConfigs(float InMaxSpeed, float InMaxAcceleration, float InBrakingDeceleration, float InGroundFriction, float InJumpZPower, float InAirControl, float InRotationInterpSpeed, const TObjectPtr<UCharacterMovementCondition>& InCondition)
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
	TObjectPtr<UCharacterMovementCondition> Condition{ nullptr };

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

public:
	/**
	 * Get a GaitTag that can be transitioned
	 */
	const FCharacterGaitConfigs& GetAllowedGait(const UGCLACharacterMovementComponent* CMC, const FGameplayTag& DesiredGait, FGameplayTag& OutTag) const;

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
	TObjectPtr<UCharacterMovementCondition> Condition;

public:
	/**
	 * Get a StanceTag that can be transposed.
	 */
	const FCharacterStanceConfigs& GetAllowedStance(const FGameplayTag& DesiredStance, FGameplayTag& OutTag) const;

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
	// Gait to be transitioned when an invalid rotation mode is set.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Categories = "Status.RotationMode"))
	FGameplayTag DefaultRotationMode;

	//
	// RotationMode allowed during this LocomotionMode is applied.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Categories = "Status.RotationMode", ForceInlineRow))
	TMap<FGameplayTag, FCharacterRotationModeConfigs> RotationModes;

public:
	/**
	 * Get a RotationModeTag that can be transposed.
	 */
	const FCharacterRotationModeConfigs& GetAllowedRotationMode(const UGCLACharacterMovementComponent* CMC, const FGameplayTag& DesiredRotationMode, FGameplayTag& OutTag) const;

};

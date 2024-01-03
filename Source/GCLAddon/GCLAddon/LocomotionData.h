// Copyright (C) 2023 owoDra

#pragma once

#include "Engine/DataAsset.h"

#include "Type/LocomotionConfigTypes.h"

#include "GameplayTagContainer.h"

#include "LocomotionData.generated.h"

class ULocomotionComponent;
class UCustomMovementProcess;
enum EMovementMode : int;


/**
 * Data asset of configuration and definition information about the character's Locomotion
 */
UCLASS(Blueprintable, BlueprintType, Const)
class GCLADDON_API ULocomotionData : public UDataAsset
{
	GENERATED_BODY()
public:
	ULocomotionData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//////////////////////////////////////////////////////////////////////////////////////////
	// General
public:
	//
	// Mapping list of LocomotionMode tags and MovementMode types for the character
	// 
	// Tips:
	//	It is used to convert between MovementMode, which identifies the character's movement process,
	//  and LocomotionMode, which describes the character's movement state.
	// 
	//	LocomotionComponent uses this data to update LocomotionMode when MovementMode is changed.
	//
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "General", Meta = (ForceInlineRow, Categories = "Status.LocomotionMode"))
	TMap<TEnumAsByte<EMovementMode>, FGameplayTag> MovementModeToLocomotionMode;

	//
	// Mapping list of LocomotionMode tags and CustomMovementMode Index for the character
	// 
	// Tips:
	//	It is used to convert between CustomMovementMode, which identifies the character's custom movement process,
	//  and LocomotionMode, which describes the character's movement state.
	// 
	//	LocomotionComponent uses this data to update LocomotionMode when CustomMovementMode is changed.
	//
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "General", Meta = (ForceInlineRow, Categories = "Status.LocomotionMode"))
	TMap<uint8, FGameplayTag> CustomMovementModeToLocomotionMode;

	//
	// Mapping list of custom movement mode indexes and classes defined for the movement process
	// 
	// Tips:
	//	When initializing LocomotionComponent, it uses this data to create an instance of the custom movement process.
	//
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "General", Meta = (ForceInlineRow))
	TMap<uint8, TSoftClassPtr<UCustomMovementProcess>> CustomMovementProcesses;

	//
	// Mapping list of LocomotionMode and LocomotionDefinition
	// 
	// Tips:
	//	LocomotionComponent updates parameters based on this data when LocomotionMode is changed.
	//
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "General", Meta = (ForceInlineRow, Categories = "Status.LocomotionMode"))
	TMap<FGameplayTag, FCharacterLocomotionModeConfigs> LocomotionModes;


	//////////////////////////////////////////////////////////////////////////////////////////
	// Default
public:
	//
	// Default Character RotationMode
	//
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Default", Meta = (ForceInlineRow, Categories = "Status.RotationMode"))
	FGameplayTag DefaultRotationMode;

	//
	// Default Character Stance
	//
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Default", Meta = (ForceInlineRow, Categories = "Status.Stance"))
	FGameplayTag DefaultStance;

	//
	// Default Character Gait
	//
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Default", Meta = (ForceInlineRow, Categories = "Status.Gait"))
	FGameplayTag DefaultGait;


	//////////////////////////////////////////////////////////////////////////////////////////
	// Movement
public:
	//
	// Threshold of speed at which it is judged to be moving
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float MovingSpeedThreshold{ 50.0f };


	//////////////////////////////////////////////////////////////////////////////////////////
	// Rotation
public:
	//
	// Whether the rotation of the object on which the character is standing should be reflected in the character
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	bool bInheritMovementBaseRotationInVelocityDirectionRotationMode{ true };

	//
	// Whether to rotate the character in the direction of movement
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	bool bRotateTowardsDesiredVelocityInVelocityDirectionRotationMode{ true };


	//////////////////////////////////////////////////////////////////////////////////////////
	// Network
public:
	//
	// Whether to use net smoothing
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Netowork")
	bool bEnableNetworkSmoothing{ true };

	//
	// Whether to listen for net smoothing from the server
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Netowork")
	bool bEnableListenServerNetworkSmoothing{ true };

public:
	/**
	 * Find LocomotionModeConfigs from DesiredLocomotionMode
	 */
	const FCharacterLocomotionModeConfigs& GetLocomotionModeConfig(const FGameplayTag& LomotionMode) const;

	/**
	 * Convert MovementMode type to LocomotionMode tag
	 */
	FGameplayTag Convert_MovementModeToLocomotionMode(const EMovementMode& MovementMode, const uint8& CustomMovementMode) const;

	/**
	 * Convert LocomotionMode tag to MovementMode type
	 * 
	 * Note:
	 *	Be aware that a single LocomotionMode can be associated with multiple MovementModes. 
	 *	For example, if "Status.LocomotionMode.InAir" is specified, it is ambiguous whether the desired MovementMode is "Falling" or "Flying".
	 */
	void Convert_LocomotionModeToMovementMode(const FGameplayTag& LocomotionMode, EMovementMode& MovementMode, uint8& CustomMovementMode) const;

	/**
	 * Determine whether a transition to the desired MovementMode is possible based on the defined data.
	 */
	bool CanChangeMovementModeTo(const ULocomotionComponent* LC, const EMovementMode& MovementMode, const uint8& CustomMovementMode) const;

};

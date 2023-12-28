// Copyright (C) 2023 owoDra

#pragma once

#include "Engine/DataAsset.h"

#include "CharacterMovementConfigs.h"

#include "GameplayTagContainer.h"

#include "CharacterMovementData.generated.h"

class UCurveFloat;
class UCurveVector;
class UAnimMontage;
class UCharacterCustomMovement;
class UGCLACharacterMovementComponent;


/**
 * Data assets that define character movement settings
 */
UCLASS(BlueprintType, Const)
class GCLADDON_API UCharacterMovementData : public UDataAsset
{
	GENERATED_BODY()
public:
	UCharacterMovementData();

public:
	//
	// Basic movement settings for each LocomotionModes
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General", Meta = (ForceInlineRow))
	TMap<FGameplayTag, FCharacterLocomotionModeConfigs> LocomotionModes;

	//
	// Movement processing in CustomMovementMode
	// 
	// Tips:
	// Array order matches CustomMovement number.
	// The basic movement settings during LocomotionMode in CustomMovement can be defined in LocomotionModes.
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General", Instanced)
	TArray<TObjectPtr<UCharacterCustomMovement>> CustomMovements;

	//
	// Threshold of speed at which it is judged to be moving
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float MovingSpeedThreshold{ 50.0f };

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
	const FCharacterLocomotionModeConfigs& GetAllowedLocomotionMode(const FGameplayTag& LomotionMode) const;

};

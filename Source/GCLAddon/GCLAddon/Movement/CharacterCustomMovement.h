// Copyright (C) 2023 owoDra

#pragma once

#include "Engine/DataAsset.h"

#include "GameplayTagContainer.h"

#include "CharacterCustomMovement.generated.h"

class UCharacterMovementComponent;


/**
 * Movement processing that can be added to CharacterMovementComponent
 * Used in CustomMovementMode
 */
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class GCLADDON_API UCharacterCustomMovement : public UObject
{
	GENERATED_BODY()
public:
	UCharacterCustomMovement(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

#if WITH_EDITORONLY_DATA
	virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) {}
#endif

protected:
	//
	// Tag for LocomotionMode of this Movement
	//
	FGameplayTag LocomotionModeTag;

public:
	/**
	 * Returns the LocomotionMode of this Movement
	 */
	const FGameplayTag& GetLocomotionMode() const { return LocomotionModeTag; }

public:
	/**
	 * The actual processing of this Movement.
	 */
	virtual void PhysMovement(UCharacterMovementComponent* CMC, float DeltaTime, int32 Iterations) {}

	/**
	 * Called when the processing of this Movement starts.
	 */
	virtual void OnMovementStart(const UCharacterMovementComponent* CMC) {}

	/**
	 * Called when this Movement finishes processing.
	 */
	virtual void OnMovementEnd(const UCharacterMovementComponent* CMC) {}

};

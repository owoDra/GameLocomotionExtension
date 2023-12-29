// Copyright (C) 2023 owoDra

#pragma once

#include "Engine/DataAsset.h"

#include "GameplayTagContainer.h"

#include "CustomMovementProcess.generated.h"

class ULocomotionComponent;


/**
 * Movement processing that can be added to LocomotionComponent
 * Used in CustomMovementMode
 */
UCLASS(Abstract)
class GCLADDON_API UCustomMovementProcess : public UObject
{
	GENERATED_BODY()
public:
	UCustomMovementProcess(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

#if WITH_EDITORONLY_DATA
	virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) {}
#endif


public:
	/**
	 * The actual processing of this Movement.
	 */
	virtual void PhysMovement(ULocomotionComponent* LC, float DeltaTime, int32 Iterations) {}

	/**
	 * Called when the processing of this Movement starts.
	 */
	virtual void OnMovementStart(const ULocomotionComponent* LC) {}

	/**
	 * Called when this Movement finishes processing.
	 */
	virtual void OnMovementEnd(const ULocomotionComponent* LC) {}

};

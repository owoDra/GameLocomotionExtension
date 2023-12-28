// Copyright (C) 2023 owoDra

#pragma once

#include "GameplayTagContainer.h"

#include "CharacterMovementCondition.generated.h"

class UGCLACharacterMovementComponent;


/**
 * Determines if it is possible to transition to a specific State used in GCLACharacterMovementComponent.
 */
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class GCLADDON_API UCharacterMovementCondition : public UObject
{
	GENERATED_BODY()
public:
	/**
	 * Determine if a transition is possible.
	 */
	virtual bool CanEnter(const UGCLACharacterMovementComponent* CMC) const { return false; }

public:
	//
	// Recommend a transition when it was not possible Tag
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Condition")
	FGameplayTag SuggestStateTag{ FGameplayTag::EmptyTag };

};

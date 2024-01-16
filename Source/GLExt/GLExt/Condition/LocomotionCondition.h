// Copyright (C) 2024 owoDra

#pragma once

#include "GameplayTagContainer.h"

#include "LocomotionCondition.generated.h"

class ULocomotionComponent;


/**
 * Determines if it is possible to transition to a specific State used in LocomotionComponent.
 */
UCLASS(Abstract, Const, DefaultToInstanced, EditInlineNew)
class GLEXT_API ULocomotionCondition : public UObject
{
	GENERATED_BODY()
public:
	/**
	 * Determine if a transition is possible.
	 */
	virtual bool CanEnter(const ULocomotionComponent* LC) const { return false; }

public:
	//
	// Recommend a transition when it was not possible Tag
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Condition")
	FGameplayTag SuggestStateTag{ FGameplayTag::EmptyTag };

};

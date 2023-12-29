// Copyright (C) 2023 owoDra

#pragma once

#include "Condition/LocomotionCondition.h"

#include "LocomotionCondition_MovingDirection.generated.h"


/**
 * Determined using viewpoint angle and angle of travel
 */
UCLASS(meta = (DisplayName = "Condition Moving Direction"))
class ULocomotionCondition_MovingDirection : public ULocomotionCondition
{
	GENERATED_BODY()
public:
	virtual bool CanEnter(const ULocomotionComponent* LC) const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Condition")
	float ViewRelativeAngleThreshold{ 50.0 };

};

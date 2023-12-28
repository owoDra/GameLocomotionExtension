// Copyright (C) 2023 owoDra

#pragma once

#include "Movement/CharacterMovementCondition.h"

#include "CharacterMovementCondition_MovingDirection.generated.h"

class UCharacterMovementComponent;


/**
 * Determined using viewpoint angle and angle of travel
 */
UCLASS(meta = (DisplayName = "Condition Moving Direction"))
class UCharacterMovementCondition_MovingDirection : public UCharacterMovementCondition
{
	GENERATED_BODY()
public:
	virtual bool CanEnter(const UGCLACharacterMovementComponent* CMC) const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Condition")
	float ViewRelativeAngleThreshold{ 50.0 };

};

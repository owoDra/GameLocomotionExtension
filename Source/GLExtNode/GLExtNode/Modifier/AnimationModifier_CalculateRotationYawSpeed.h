#pragma once

#include "AnimationModifier.h"

#include "AnimationModifier_CalculateRotationYawSpeed.generated.h"

/**
 * This AnimationModifier is used to calculate the root rotation speed and create a rotation speed curve for the rotation Yaw of the rotation animation.
 */
UCLASS(DisplayName = "Calculate Rotation Yaw Speed Animation Modifier")
class GLEXTNODE_API UAnimationModifier_CalculateRotationYawSpeed : public UAnimationModifier
{
public:
	GENERATED_BODY()

public:
	virtual void OnApply_Implementation(UAnimSequence* Sequence) override;

};

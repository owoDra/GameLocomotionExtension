// Copyright (C) 2024 owoDra

#include "LocomotionCondition_MovingDirection.h"

#include "LocomotionComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LocomotionCondition_MovingDirection)


bool ULocomotionCondition_MovingDirection::CanEnter(const ULocomotionComponent* LC) const
{
	if (FMath::Abs(FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(
		LC->GetLocomotionState().InputYawAngle - LC->GetViewState().Rotation.Yaw))) < ViewRelativeAngleThreshold)
	{
		return true;
	}

	return false;
}

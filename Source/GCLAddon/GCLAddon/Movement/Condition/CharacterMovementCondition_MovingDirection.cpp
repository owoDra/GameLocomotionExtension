// Copyright (C) 2023 owoDra

#include "CharacterMovementCondition_MovingDirection.h"

#include "GCLACharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterMovementCondition_MovingDirection)


bool UCharacterMovementCondition_MovingDirection::CanEnter(const UGCLACharacterMovementComponent* CMC) const
{
	if (FMath::Abs(FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(
		CMC->GetLocomotionState().InputYawAngle - CMC->GetViewState().Rotation.Yaw))) < ViewRelativeAngleThreshold)
	{
		return true;
	}

	return false;
}

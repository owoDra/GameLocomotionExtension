// Copyright (C) 2024 owoDra

#pragma once

#include "NativeGameplayTags.h"


////////////////////////////////////
// Status.LocomotionMode

GCLADDON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_LocomotionMode_OnGround);
GCLADDON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_LocomotionMode_InAir);
GCLADDON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_LocomotionMode_InWater);


////////////////////////////////////
// Status.RotationMode

GCLADDON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_RotationMode_VelocityDirection);
GCLADDON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_RotationMode_ViewDirection);
GCLADDON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_RotationMode_Aiming);


////////////////////////////////////
// Status.Stance

GCLADDON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Stance_Standing);
GCLADDON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Stance_Crouching);


////////////////////////////////////
// Status.Gait

GCLADDON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Gait_Walking);
GCLADDON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Gait_Running);
GCLADDON_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Gait_Sprinting);

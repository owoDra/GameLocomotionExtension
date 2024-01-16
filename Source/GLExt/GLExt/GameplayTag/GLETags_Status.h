// Copyright (C) 2024 owoDra

#pragma once

#include "NativeGameplayTags.h"


////////////////////////////////////
// Status.LocomotionMode

GLEXT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_LocomotionMode_OnGround);
GLEXT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_LocomotionMode_InAir);
GLEXT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_LocomotionMode_InWater);


////////////////////////////////////
// Status.RotationMode

GLEXT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_RotationMode_VelocityDirection);
GLEXT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_RotationMode_ViewDirection);
GLEXT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_RotationMode_Aiming);


////////////////////////////////////
// Status.Stance

GLEXT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Stance_Standing);
GLEXT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Stance_Crouching);


////////////////////////////////////
// Status.Gait

GLEXT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Gait_Walking);
GLEXT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Gait_Running);
GLEXT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Gait_Sprinting);

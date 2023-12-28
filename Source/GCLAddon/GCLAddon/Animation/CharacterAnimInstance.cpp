// Copyright (C) 2023 owoDra

#include "CharacterAnimInstance.h"

#include "Animation/CharacterAnimInstanceProxy.h"
#include "Animation/CharacterAnimData.h"
#include "Animation/CharacterAnimCurveNameStatics.h"
#include "Movement/MovementMathLibrary.h"
#include "GameplayTag/GCLATags_Status.h"
#include "GCLACharacterMovementComponent.h"
#include "GCLACharacter.h"
#include "GCLAddonStatGroup.h"

#include "Components/CapsuleComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterAnimInstance)


UCharacterAnimInstance::UCharacterAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
}

void UCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<AGCLACharacter>(GetOwningActor());

#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld() && !IsValid(Character))
	{
		// Use CDO of Character for display in editor

		Character = GetMutableDefault<AGCLACharacter>();
	}
#endif

	CharacterMovement = Cast<UGCLACharacterMovementComponent>(Character->GetCharacterMovement());
}

void UCharacterAnimInstance::NativeBeginPlay()
{
	ensureMsgf(IsValid(AnimData)			, TEXT("UCharacterAnimInstance::NativeBeginPlay: Invalid AnimData"));
	ensureMsgf(IsValid(Character)			, TEXT("UCharacterAnimInstance::NativeBeginPlay: Invalid Character"));
	ensureMsgf(IsValid(CharacterMovement)	, TEXT("UCharacterAnimInstance::NativeBeginPlay: Invalid CharacterMovement"));
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UCharacterAnimInstance::NativeUpdateAnimation()"), STAT_UCharacterAnimInstance_NativeUpdateAnimation, STATGROUP_GCLAMovement);

	Super::NativeUpdateAnimation(DeltaTime);

	if (!IsValid(AnimData) || !IsValid(Character) || !IsValid(CharacterMovement))
	{
		return;
	}

	if (GetSkelMeshComponent()->IsUsingAbsoluteRotation())
	{
		const auto& ActorTransform{ Character->GetActorTransform() };

		// Manual synchronization of mesh rotation with character rotation

		GetSkelMeshComponent()->MoveComponent(FVector::ZeroVector, ActorTransform.GetRotation() * Character->GetBaseRotationOffset(), false);

		// Re-cache proxy transforms to match the modified mesh transform.

		const auto& Proxy{ GetProxyOnGameThread<FAnimInstanceProxy>() };

		const_cast<FTransform&>(Proxy.GetComponentTransform())			= GetSkelMeshComponent()->GetComponentTransform();
		const_cast<FTransform&>(Proxy.GetComponentRelativeTransform())	= GetSkelMeshComponent()->GetRelativeTransform();
		const_cast<FTransform&>(Proxy.GetActorTransform())				= ActorTransform;
	}

	LocomotionMode		= CharacterMovement->GetLocomotionMode();
	RotationMode		= CharacterMovement->GetRotationMode();
	Stance				= CharacterMovement->GetStance();
	Gait				= CharacterMovement->GetGait();
	LocomotionAction	= CharacterMovement->GetLocomotionAction();

	UpdateMovementBaseOnGameThread();
	UpdateViewOnGameThread();
	UpdateLocomotionOnGameThread();
	UpdateGroundedOnGameThread();
	UpdateInAirOnGameThread();

	UpdateFeetOnGameThread();
}

void UCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UCharacterAnimInstance::NativeThreadSafeUpdateAnimation()"), STAT_UCharacterAnimInstance_NativeThreadSafeUpdateAnimation, STATGROUP_GCLAMovement);

	Super::NativeThreadSafeUpdateAnimation(DeltaTime);

	if (!IsValid(AnimData) || !IsValid(Character) || !IsValid(CharacterMovement))
	{
		return;
	}

	UpdateLayering();
	UpdatePose();

	UpdateView(DeltaTime);
	UpdateGrounded(DeltaTime);
	UpdateInAir(DeltaTime);

	UpdateFeet(DeltaTime);

	UpdateTransitions();
	UpdateRotateInPlace(DeltaTime);
}

void UCharacterAnimInstance::NativePostEvaluateAnimation()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UCharacterAnimInstance::NativePostEvaluateAnimation()"), STAT_UCharacterAnimInstance_NativePostEvaluateAnimation, STATGROUP_GCLAMovement)

	Super::NativePostEvaluateAnimation();

	if (!IsValid(AnimData) || !IsValid(Character) || !IsValid(CharacterMovement))
	{
		return;
	}

	PlayQueuedDynamicTransitionAnimation();

	bPendingUpdate = false;
}

FAnimInstanceProxy* UCharacterAnimInstance::CreateAnimInstanceProxy()
{
	return new FCharacterAnimInstanceProxy(this);
}


FControlRigInput UCharacterAnimInstance::GetControlRigInput() const
{
	return {
		(!IsValid(AnimData) || AnimData->General.bUseHandIkBones),
		(!IsValid(AnimData) || AnimData->General.bUseFootIkBones),
		GroundedState.VelocityBlend.ForwardAmount,
		GroundedState.VelocityBlend.BackwardAmount,
		ViewState.SpineRotation.YawAngle,
		FeetState.Left.IkRotation,
		FeetState.Left.IkLocation,
		FeetState.Left.IkAmount,
		FeetState.Right.IkRotation,
		FeetState.Right.IkLocation,
		FeetState.Right.IkAmount,
		FeetState.MinMaxPelvisOffsetZ,
	};
}

void UCharacterAnimInstance::MarkTeleported()
{
	TeleportedTime = GetWorld()->GetTimeSeconds();
}

void UCharacterAnimInstance::UpdateMovementBaseOnGameThread()
{
	const auto& BasedMovement{ Character->GetBasedMovement() };

	if (BasedMovement.MovementBase != MovementBase.Primitive || BasedMovement.BoneName != MovementBase.BoneName)
	{
		MovementBase.Primitive = BasedMovement.MovementBase;
		MovementBase.BoneName = BasedMovement.BoneName;
		MovementBase.bBaseChanged = true;
	}
	else
	{
		MovementBase.bBaseChanged = false;
	}

	MovementBase.bHasRelativeLocation = BasedMovement.HasRelativeLocation();
	MovementBase.bHasRelativeRotation = MovementBase.bHasRelativeLocation && BasedMovement.bRelativeRotation;

	const auto PreviousRotation{ MovementBase.Rotation };

	MovementBaseUtility::GetMovementBaseTransform(BasedMovement.MovementBase, BasedMovement.BoneName, MovementBase.Location, MovementBase.Rotation);

	MovementBase.DeltaRotation = (MovementBase.bHasRelativeLocation && !MovementBase.bBaseChanged)
								? (MovementBase.Rotation * PreviousRotation.Inverse()).Rotator()
								: FRotator::ZeroRotator;
}

void UCharacterAnimInstance::UpdateLayering()
{
	const auto& Curves{ GetProxyOnAnyThread<FCharacterAnimInstanceProxy>().GetAnimationCurves(EAnimCurveType::AttributeCurve) };

	// Lambda function to get the value of AnimCurve

	static const auto GetCurveValue
	{
		[](const TMap<FName, float>& Curves, const FName& CurveName) -> float
		{
			const float* Value{Curves.Find(CurveName)};

			return (Value != nullptr) ? *Value : 0.0f;
		}
	};

	LayeringState.HeadBlendAmount				= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerHeadCurveName());
	LayeringState.HeadAdditiveBlendAmount		= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerHeadAdditiveCurveName());
	LayeringState.HeadSlotBlendAmount			= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerHeadSlotCurveName());

	LayeringState.ArmLeftBlendAmount			= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerArmLeftCurveName());
	LayeringState.ArmLeftAdditiveBlendAmount	= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerArmLeftAdditiveCurveName());
	LayeringState.ArmLeftSlotBlendAmount		= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerArmLeftSlotCurveName());
	LayeringState.ArmLeftLocalSpaceBlendAmount	= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerArmLeftLocalSpaceCurveName());
	LayeringState.ArmLeftMeshSpaceBlendAmount	= !FAnimWeight::IsFullWeight(LayeringState.ArmLeftLocalSpaceBlendAmount);

	LayeringState.ArmRightBlendAmount			= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerArmRightCurveName());
	LayeringState.ArmRightAdditiveBlendAmount	= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerArmRightAdditiveCurveName());
	LayeringState.ArmRightSlotBlendAmount		= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerArmRightSlotCurveName());
	LayeringState.ArmRightLocalSpaceBlendAmount = GetCurveValue(Curves, UCharacterAnimCurveNames::LayerArmRightLocalSpaceCurveName());
	LayeringState.ArmRightMeshSpaceBlendAmount	= !FAnimWeight::IsFullWeight(LayeringState.ArmRightLocalSpaceBlendAmount);

	LayeringState.HandLeftBlendAmount			= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerHandLeftCurveName());
	LayeringState.HandRightBlendAmount			= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerHandRightCurveName());

	LayeringState.SpineBlendAmount				= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerSpineCurveName());
	LayeringState.SpineAdditiveBlendAmount		= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerSpineAdditiveCurveName());
	LayeringState.SpineSlotBlendAmount			= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerSpineSlotCurveName());

	LayeringState.PelvisBlendAmount				= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerPelvisCurveName());
	LayeringState.PelvisSlotBlendAmount			= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerPelvisSlotCurveName());

	LayeringState.LegsBlendAmount				= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerLegsCurveName());
	LayeringState.LegsSlotBlendAmount			= GetCurveValue(Curves, UCharacterAnimCurveNames::LayerLegsSlotCurveName());
}

void UCharacterAnimInstance::UpdatePose()
{
	const auto& Curves{ GetProxyOnAnyThread<FCharacterAnimInstanceProxy>().GetAnimationCurves(EAnimCurveType::AttributeCurve) };

	// Lambda function to get the value of AnimCurve

	static const auto GetCurveValue
	{
		[](const TMap<FName, float>& Curves, const FName& CurveName) -> float
		{
			const float* Value{Curves.Find(CurveName)};

			return (Value != nullptr) ? *Value : 0.0f;
		}
	};

	PoseState.GroundedAmount		= GetCurveValue(Curves, UCharacterAnimCurveNames::PoseGroundedCurveName());
	PoseState.InAirAmount			= GetCurveValue(Curves, UCharacterAnimCurveNames::PoseInAirCurveName());

	PoseState.StandingAmount		= GetCurveValue(Curves, UCharacterAnimCurveNames::PoseStandingCurveName());
	PoseState.CrouchingAmount		= GetCurveValue(Curves, UCharacterAnimCurveNames::PoseCrouchingCurveName());

	PoseState.MovingAmount			= GetCurveValue(Curves, UCharacterAnimCurveNames::PoseMovingCurveName());

	PoseState.GaitAmount			= FMath::Clamp(GetCurveValue(Curves, UCharacterAnimCurveNames::PoseGaitCurveName()), 0.0f, 3.0f);
	PoseState.GaitWalkingAmount		= UMovementMathLibrary::Clamp01(PoseState.GaitAmount);
	PoseState.GaitRunningAmount		= UMovementMathLibrary::Clamp01(PoseState.GaitAmount - 1.0f);
	PoseState.GaitSprintingAmount	= UMovementMathLibrary::Clamp01(PoseState.GaitAmount - 2.0f);

	// Unweight" the Walk Pose curve using the value of the Ground Pose curve
	// It instantly retrieves the full yield value from the beginning of the transition to the ground state

	PoseState.UnweightedGaitAmount	= (PoseState.GroundedAmount > 0.0f) ? (PoseState.GaitAmount / PoseState.GroundedAmount) : PoseState.GaitAmount;

	PoseState.UnweightedGaitWalkingAmount	= UMovementMathLibrary::Clamp01(PoseState.UnweightedGaitAmount);
	PoseState.UnweightedGaitRunningAmount	= UMovementMathLibrary::Clamp01(PoseState.UnweightedGaitAmount - 1.0f);
	PoseState.UnweightedGaitSprintingAmount = UMovementMathLibrary::Clamp01(PoseState.UnweightedGaitAmount - 2.0f);
}


#pragma region View

bool UCharacterAnimInstance::IsSpineRotationAllowed()
{
	return RotationMode != TAG_Status_RotationMode_VelocityDirection;
}

void UCharacterAnimInstance::UpdateViewOnGameThread()
{
	check(IsInGameThread());

	const auto& View{ CharacterMovement->GetViewState() };

	ViewState.Rotation = View.Rotation;
	ViewState.YawSpeed = View.YawSpeed;
}

void UCharacterAnimInstance::UpdateView(float DeltaTime)
{
	if (!LocomotionAction.IsValid())
	{
		ViewState.YawAngle		= FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw - LocomotionState.Rotation.Yaw));
		ViewState.PitchAngle	= FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Pitch - LocomotionState.Rotation.Pitch));

		ViewState.PitchAmount	= (0.5f - ViewState.PitchAngle / 180.0f);
	}

	const auto ViewAmount{ 1.0f - GetCurveValueClamped01(UCharacterAnimCurveNames::ViewBlockCurveName()) };
	const auto AimingAmount{ GetCurveValueClamped01(UCharacterAnimCurveNames::AllowAimingCurveName()) };

	ViewState.LookAmount		= (ViewAmount * (1.0f - AimingAmount));

	UpdateSpineRotation(DeltaTime);

	ViewState.SpineRotation.YawAngle *= (ViewAmount * AimingAmount);
}

void UCharacterAnimInstance::UpdateSpineRotation(float DeltaTime)
{
	auto& SpineRotation{ ViewState.SpineRotation };

	if (SpineRotation.bSpineRotationAllowed != IsSpineRotationAllowed())
	{
		SpineRotation.bSpineRotationAllowed = !SpineRotation.bSpineRotationAllowed;
		SpineRotation.StartYawAngle = SpineRotation.CurrentYawAngle;
	}

	if (SpineRotation.bSpineRotationAllowed)
	{
		static constexpr auto InterpolationSpeed{ 20.0f };

		SpineRotation.SpineAmount = bPendingUpdate ? 1.0f : UMovementMathLibrary::ExponentialDecay(SpineRotation.SpineAmount, 1.0f, DeltaTime, InterpolationSpeed);

		SpineRotation.TargetYawAngle = ViewState.YawAngle;
	}
	else
	{
		static constexpr auto InterpolationSpeed{ 10.0f };

		SpineRotation.SpineAmount = bPendingUpdate ? 0.0f : UMovementMathLibrary::ExponentialDecay(SpineRotation.SpineAmount, 0.0f, DeltaTime, InterpolationSpeed);
	}

	SpineRotation.CurrentYawAngle = UMovementMathLibrary::LerpAngle(SpineRotation.StartYawAngle, SpineRotation.TargetYawAngle, SpineRotation.SpineAmount);

	SpineRotation.YawAngle = SpineRotation.CurrentYawAngle;
}

void UCharacterAnimInstance::ReinitializeLook()
{
	ViewState.Look.bReinitializationRequired = true;
}

void UCharacterAnimInstance::UpdateLook()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UCharacterAnimInstance::UpdateLook()"), STAT_UCharacterAnimInstance_UpdateLook, STATGROUP_GCLAMovement)

	if (!IsValid(AnimData))
	{
		return;
	}

	auto& Look{ ViewState.Look };

	Look.bReinitializationRequired |= bPendingUpdate;

	const auto CharacterYawAngle{ UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw) };

	if (MovementBase.bHasRelativeRotation)
	{
		// Offset angles to maintain angles relative to the moving base

		Look.WorldYawAngle = FRotator3f::NormalizeAxis(Look.WorldYawAngle + MovementBase.DeltaRotation.Yaw);
	}

	float TargetYawAngle;
	float TargetPitchAngle;
	float InterpolationSpeed;

	if (RotationMode == TAG_Status_RotationMode_VelocityDirection)
	{
		// Try to look in the input direction

		TargetYawAngle = FRotator3f::NormalizeAxis((LocomotionState.bHasInput ? LocomotionState.InputYawAngle : LocomotionState.TargetYawAngle) - CharacterYawAngle);

		TargetPitchAngle = 0.0f;
		InterpolationSpeed = AnimData->View.LookTowardsInputYawAngleInterpolationSpeed;
	}
	else
	{
		// Try to look in the direction of the view

		TargetYawAngle = ViewState.YawAngle;
		TargetPitchAngle = ViewState.PitchAngle;
		InterpolationSpeed = AnimData->View.LookTowardsCameraRotationInterpolationSpeed;
	}

	if (Look.bReinitializationRequired || InterpolationSpeed <= 0.0f)
	{
		Look.YawAngle = TargetYawAngle;
		Look.PitchAngle = TargetPitchAngle;
	}
	else
	{
		const auto YawAngle{ FRotator3f::NormalizeAxis(Look.WorldYawAngle - CharacterYawAngle) };
		auto DeltaYawAngle{ FRotator3f::NormalizeAxis(TargetYawAngle - YawAngle) };

		if (DeltaYawAngle > 180.0f - UMovementMathLibrary::CounterClockwiseRotationAngleThreshold)
		{
			DeltaYawAngle -= 360.0f;
		}
		else if (FMath::Abs(LocomotionState.YawSpeed) > UE_SMALL_NUMBER && FMath::Abs(TargetYawAngle) > 90.0f)
		{
			// When interpolating yaw angle, priority is given to the direction of rotation of the character rather than the shortest direction of rotation.
			// Ensure that the rotation of the head and the body are in sync.

			DeltaYawAngle = LocomotionState.YawSpeed > 0.0f ? FMath::Abs(DeltaYawAngle) : -FMath::Abs(DeltaYawAngle);
		}

		const auto InterpolationAmount{ UMovementMathLibrary::ExponentialDecay(GetDeltaSeconds(), InterpolationSpeed) };

		Look.YawAngle = FRotator3f::NormalizeAxis(YawAngle + DeltaYawAngle * InterpolationAmount);
		Look.PitchAngle = UMovementMathLibrary::LerpAngle(Look.PitchAngle, TargetPitchAngle, InterpolationAmount);
	}

	Look.WorldYawAngle = FRotator3f::NormalizeAxis(CharacterYawAngle + Look.YawAngle);

	// The yaw angle is divided into three separate values. These three values are used to improve the blending of the view when the character is rotated completely around.
	// It is used to improve the blending of views when rotating completely around a character.
	// This allows for smooth blending from left to right or right to left while maintaining view responsiveness.

	Look.YawForwardAmount = Look.YawAngle / 360.0f + 0.5f;
	Look.YawLeftAmount = 0.5f - FMath::Abs(Look.YawForwardAmount - 0.5f);
	Look.YawRightAmount = 0.5f + FMath::Abs(Look.YawForwardAmount - 0.5f);

	Look.bReinitializationRequired = false;
}

#pragma endregion


#pragma region Locomotion

void UCharacterAnimInstance::UpdateLocomotionOnGameThread()
{
	check(IsInGameThread());

	const auto& Locomotion{ CharacterMovement->GetLocomotionState() };

	LocomotionState.bHasInput				= Locomotion.bHasInput;
	LocomotionState.InputYawAngle			= Locomotion.InputYawAngle;

	LocomotionState.Speed					= Locomotion.Speed;
	LocomotionState.Velocity				= Locomotion.Velocity;
	LocomotionState.VelocityYawAngle		= Locomotion.VelocityYawAngle;
	LocomotionState.Acceleration			= Locomotion.Acceleration;

	LocomotionState.MaxAcceleration			= CharacterMovement->GetMaxAcceleration();
	LocomotionState.MaxBrakingDeceleration	= CharacterMovement->GetMaxBrakingDeceleration();
	LocomotionState.WalkableFloorZ			= CharacterMovement->GetWalkableFloorZ();

	LocomotionState.bMoving					= Locomotion.bMoving;

	LocomotionState.bMovingSmooth			= (Locomotion.bHasInput && Locomotion.bHasSpeed) || (Locomotion.Speed > AnimData->General.MovingSmoothSpeedThreshold);

	LocomotionState.TargetYawAngle			= Locomotion.TargetYawAngle;
	LocomotionState.Location				= Locomotion.Location;
	LocomotionState.Rotation				= Locomotion.Rotation;
	LocomotionState.RotationQuaternion		= Locomotion.RotationQuaternion;
	LocomotionState.YawSpeed				= Locomotion.YawSpeed;

	LocomotionState.Scale					= UE_REAL_TO_FLOAT(GetSkelMeshComponent()->GetComponentScale().Z);

	const auto* Capsule{ Character->GetCapsuleComponent() };

	LocomotionState.CapsuleRadius			= Capsule->GetScaledCapsuleRadius();
	LocomotionState.CapsuleHalfHeight		= Capsule->GetScaledCapsuleHalfHeight();
}

#pragma endregion


#pragma region OnGround

void UCharacterAnimInstance::SetHipsDirection(EHipsDirection NewHipsDirection)
{
	GroundedState.HipsDirection = NewHipsDirection;
}

void UCharacterAnimInstance::ActivatePivot()
{
	GroundedState.bPivotActivationRequested = true;
}

void UCharacterAnimInstance::UpdateGroundedOnGameThread()
{
	check(IsInGameThread());

	GroundedState.bPivotActive = GroundedState.bPivotActivationRequested && !bPendingUpdate && (LocomotionState.Speed < AnimData->Grounded.PivotActivationSpeedThreshold);

	GroundedState.bPivotActivationRequested = false;
}

void UCharacterAnimInstance::UpdateGrounded(float DeltaTime)
{
	// Always sample the sprint block curve. Failure to do so may cause problems related to inertial blending.

	GroundedState.SprintBlockAmount = GetCurveValueClamped01(UCharacterAnimCurveNames::SprintBlockCurveName());
	GroundedState.HipsDirectionLockAmount = FMath::Clamp(GetCurveValue(UCharacterAnimCurveNames::HipsDirectionLockCurveName()), -1.0f, 1.0f);

	if (LocomotionMode != TAG_Status_LocomotionMode_OnGround)
	{
		GroundedState.VelocityBlend.bReinitializationRequired = true;
		GroundedState.SprintTime = 0.0f;
		return;
	}

	if (!LocomotionState.bMoving)
	{
		ResetGroundedLeanAmount(DeltaTime);
		return;
	}

	// Calculate relative acceleration
	// Indicates the current amount of acceleration/deceleration relative to the character's rotation
	// This value is normalized from -1 to 1, where -1 is the maximum brake deceleration. 
	// 1 equals the maximum acceleration of the character movement component.

	FVector3f RelativeAccelerationAmount;

	if ((LocomotionState.Acceleration | LocomotionState.Velocity) >= 0.0f)
	{
		RelativeAccelerationAmount = UMovementMathLibrary::ClampMagnitude01(
			FVector3f(LocomotionState.RotationQuaternion.UnrotateVector(LocomotionState.Acceleration)) / LocomotionState.MaxAcceleration);
	}
	else
	{
		RelativeAccelerationAmount = UMovementMathLibrary::ClampMagnitude01(
			FVector3f(LocomotionState.RotationQuaternion.UnrotateVector(LocomotionState.Acceleration)) / LocomotionState.MaxBrakingDeceleration);
	}

	UpdateMovementDirection();
	UpdateVelocityBlend(DeltaTime);
	UpdateRotationYawOffsets();

	UpdateSprint(RelativeAccelerationAmount, DeltaTime);

	UpdateStrideBlendAmount();
	UpdateWalkRunBlendAmount();

	UpdateStandingPlayRate();
	UpdateCrouchingPlayRate();

	UpdateGroundedLeanAmount(RelativeAccelerationAmount, DeltaTime);
}

void UCharacterAnimInstance::UpdateMovementDirection()
{
	// Calculates the direction of movement. 
	// This value represents the character's direction of movement relative to the camera.
	// Used in cycle blending to blend to the appropriate directional state.

	if (Gait == TAG_Status_Gait_Sprinting)
	{
		GroundedState.MovementDirection = EMovementDirection::Forward;
		return;
	}

	static constexpr auto ForwardHalfAngle{ 70.0f };

	GroundedState.MovementDirection = UMovementMathLibrary::CalculateMovementDirection(
		FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(LocomotionState.VelocityYawAngle - ViewState.Rotation.Yaw)),
		ForwardHalfAngle, 5.0f);
}

void UCharacterAnimInstance::UpdateVelocityBlend(float DeltaTime)
{
	GroundedState.VelocityBlend.bReinitializationRequired |= bPendingUpdate;

	// Calculate and interpolate the amount of velocity blending 
	// This value represents the amount of velocity of the character in each direction
	// Used in blend multi-nodes to produce better directional blends than standard blend spaces

	const auto RelativeVelocityDirection{ FVector3f{LocomotionState.RotationQuaternion.UnrotateVector(LocomotionState.Velocity)}.GetSafeNormal() };

	const auto RelativeDirection{ RelativeVelocityDirection / (FMath::Abs(RelativeVelocityDirection.X) + FMath::Abs(RelativeVelocityDirection.Y) + FMath::Abs(RelativeVelocityDirection.Z)) };

	if (GroundedState.VelocityBlend.bReinitializationRequired)
	{
		GroundedState.VelocityBlend.bReinitializationRequired = false;

		GroundedState.VelocityBlend.ForwardAmount	= UMovementMathLibrary::Clamp01(RelativeDirection.X);
		GroundedState.VelocityBlend.BackwardAmount	= FMath::Abs(FMath::Clamp(RelativeDirection.X, -1.0f, 0.0f));
		GroundedState.VelocityBlend.LeftAmount		= FMath::Abs(FMath::Clamp(RelativeDirection.Y, -1.0f, 0.0f));
		GroundedState.VelocityBlend.RightAmount		= UMovementMathLibrary::Clamp01(RelativeDirection.Y);
	}
	else
	{
		GroundedState.VelocityBlend.ForwardAmount = FMath::FInterpTo(GroundedState.VelocityBlend.ForwardAmount,
			UMovementMathLibrary::Clamp01(RelativeDirection.X), DeltaTime,
			AnimData->Grounded.VelocityBlendInterpolationSpeed);

		GroundedState.VelocityBlend.BackwardAmount = FMath::FInterpTo(GroundedState.VelocityBlend.BackwardAmount,
			FMath::Abs(FMath::Clamp(RelativeDirection.X, -1.0f, 0.0f)), DeltaTime,
			AnimData->Grounded.VelocityBlendInterpolationSpeed);

		GroundedState.VelocityBlend.LeftAmount = FMath::FInterpTo(GroundedState.VelocityBlend.LeftAmount,
			FMath::Abs(FMath::Clamp(RelativeDirection.Y, -1.0f, 0.0f)), DeltaTime,
			AnimData->Grounded.VelocityBlendInterpolationSpeed);

		GroundedState.VelocityBlend.RightAmount = FMath::FInterpTo(GroundedState.VelocityBlend.RightAmount,
			UMovementMathLibrary::Clamp01(RelativeDirection.Y), DeltaTime,
			AnimData->Grounded.VelocityBlendInterpolationSpeed);
	}
}

void UCharacterAnimInstance::UpdateRotationYawOffsets()
{
	// Sets the rotational yaw offset. These values affect the rotational yaw offset curve.
	// An animation graph, used to offset the character's rotation for more natural motion.
	// The curves allow fine control of the offset behavior for each movement direction.

	const auto RotationYawOffset{ FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(LocomotionState.VelocityYawAngle - ViewState.Rotation.Yaw)) };

	GroundedState.RotationYawOffsets.ForwardAngle	= AnimData->Grounded.RotationYawOffsetForwardCurve->GetFloatValue(RotationYawOffset);
	GroundedState.RotationYawOffsets.BackwardAngle	= AnimData->Grounded.RotationYawOffsetBackwardCurve->GetFloatValue(RotationYawOffset);
	GroundedState.RotationYawOffsets.LeftAngle		= AnimData->Grounded.RotationYawOffsetLeftCurve->GetFloatValue(RotationYawOffset);
	GroundedState.RotationYawOffsets.RightAngle		= AnimData->Grounded.RotationYawOffsetRightCurve->GetFloatValue(RotationYawOffset);
}

void UCharacterAnimInstance::UpdateSprint(const FVector3f& RelativeAccelerationAmount, float DeltaTime)
{
	if (Gait != TAG_Status_Gait_Sprinting)
	{
		GroundedState.SprintTime = 0.0f;
		GroundedState.SprintAccelerationAmount = 0.0f;
		return;
	}

	// If less than 0.5 seconds, the relative acceleration is used as the sprint relative acceleration.
	// The sprint has elapsed from the start of the sprint. Otherwise, set the relative acceleration for the sprint to zero.
	// This is necessary to apply the acceleration animation only at the start of the sprint.

	static constexpr auto TimeThreshold{ 0.5f };

	GroundedState.SprintTime = bPendingUpdate ? TimeThreshold : (GroundedState.SprintTime + DeltaTime);
	GroundedState.SprintAccelerationAmount = (GroundedState.SprintTime >= TimeThreshold) ? 0.0f : RelativeAccelerationAmount.X;
}

void UCharacterAnimInstance::UpdateStrideBlendAmount()
{
	// Calculates the amount of stride blend. This value is used to scale the stride (distance the feet travel) within the blend space.
	// Allows characters to walk or run at various movement speeds. It also allows for walking and running gait animations.
	// Blends animation speeds individually while matching the speed of movement, preventing the character from needing to
	// Play a blend of half walk + half run. Curves are used to map stride volume to velocity for maximum control.

	const auto Speed{ LocomotionState.Speed / LocomotionState.Scale };

	const auto StandingStrideBlend{ FMath::Lerp(AnimData->Grounded.StrideBlendAmountWalkCurve->GetFloatValue(Speed),
												  AnimData->Grounded.StrideBlendAmountRunCurve->GetFloatValue(Speed),
												  PoseState.UnweightedGaitRunningAmount)
	};

	// The amount of blend in the crouched stride.

	GroundedState.StrideBlendAmount = FMath::Lerp(StandingStrideBlend, AnimData->Grounded.StrideBlendAmountWalkCurve->GetFloatValue(Speed), PoseState.CrouchingAmount);
}

void UCharacterAnimInstance::UpdateWalkRunBlendAmount()
{
	// Calculates the Walk / Run blend volume. This value is used within the blend space to blend walking and running.

	GroundedState.WalkRunBlendAmount = (Gait == TAG_Status_Gait_Walking) ? 0.0f : 1.0f;
}

void UCharacterAnimInstance::UpdateStandingPlayRate()
{
	// Calculate the standing play rate by dividing the character's speed by the animated speed of each walk.

	const auto WalkRunSpeedAmount{ FMath::Lerp(LocomotionState.Speed / AnimData->Grounded.AnimatedWalkSpeed,
												 LocomotionState.Speed / AnimData->Grounded.AnimatedRunSpeed,
												 PoseState.UnweightedGaitRunningAmount)
	};

	const auto WalkRunSprintSpeedAmount{ FMath::Lerp(WalkRunSpeedAmount,
													   LocomotionState.Speed / AnimData->Grounded.AnimatedSprintSpeed,
													   PoseState.UnweightedGaitSprintingAmount)
	};

	GroundedState.StandingPlayRate = FMath::Clamp(WalkRunSprintSpeedAmount / (GroundedState.StrideBlendAmount * LocomotionState.Scale), 0.0f, 3.0f);
}

void UCharacterAnimInstance::UpdateCrouchingPlayRate()
{
	GroundedState.CrouchingPlayRate = FMath::Clamp(LocomotionState.Speed / (AnimData->Grounded.AnimatedCrouchSpeed * GroundedState.StrideBlendAmount * LocomotionState.Scale), 0.0f, 2.0f);
}

void UCharacterAnimInstance::UpdateGroundedLeanAmount(const FVector3f& RelativeAccelerationAmount, float DeltaTime)
{
	if (bPendingUpdate)
	{
		LeanState.RightAmount = RelativeAccelerationAmount.Y;
		LeanState.ForwardAmount = RelativeAccelerationAmount.X;
	}
	else
	{
		LeanState.RightAmount = FMath::FInterpTo(LeanState.RightAmount, RelativeAccelerationAmount.Y, DeltaTime, AnimData->General.LeanInterpolationSpeed);
		LeanState.ForwardAmount = FMath::FInterpTo(LeanState.ForwardAmount, RelativeAccelerationAmount.X, DeltaTime, AnimData->General.LeanInterpolationSpeed);
	}
}

void UCharacterAnimInstance::ResetGroundedLeanAmount(float DeltaTime)
{
	if (bPendingUpdate)
	{
		LeanState.RightAmount = 0.0f;
		LeanState.ForwardAmount = 0.0f;
	}
	else
	{
		LeanState.RightAmount = FMath::FInterpTo(LeanState.RightAmount, 0.0f, DeltaTime, AnimData->General.LeanInterpolationSpeed);
		LeanState.ForwardAmount = FMath::FInterpTo(LeanState.ForwardAmount, 0.0f, DeltaTime, AnimData->General.LeanInterpolationSpeed);
	}
}

#pragma endregion


#pragma region InAir

void UCharacterAnimInstance::ResetJumped()
{
	InAirState.bJumped = false;
}

void UCharacterAnimInstance::UpdateInAirOnGameThread()
{
	check(IsInGameThread());

	InAirState.bJumped = !bPendingUpdate && (InAirState.bJumped || (InAirState.VerticalVelocity > 0));
}

void UCharacterAnimInstance::UpdateInAir(float DeltaTime)
{
	if (LocomotionMode != TAG_Status_LocomotionMode_InAir)
	{
		return;
	}

	if (InAirState.bJumped)
	{
		static constexpr auto ReferenceSpeed{ 600.0f };
		static constexpr auto MinPlayRate{ 1.2f };
		static constexpr auto MaxPlayRate{ 1.5f };

		InAirState.JumpPlayRate = UMovementMathLibrary::LerpClamped(MinPlayRate, MaxPlayRate, LocomotionState.Speed / ReferenceSpeed);
	}

	// Caches the vertical velocity and determines the speed at which the character lands on the ground

	InAirState.VerticalVelocity = UE_REAL_TO_FLOAT(LocomotionState.Velocity.Z);

	UpdateGroundPredictionAmount();

	UpdateInAirLeanAmount(DeltaTime);
}

void UCharacterAnimInstance::UpdateGroundPredictionAmount()
{
	// Calculate the predicted weight of the ground by tracing in the direction of velocity and finding a surface on which the character can walk.

	static constexpr auto VerticalVelocityThreshold{ -200.0f };
	static constexpr auto MinVerticalVelocity{ -4000.0f };
	static constexpr auto MaxVerticalVelocity{ -200.0f };
	static constexpr auto MinSweepDistance{ 150.0f };
	static constexpr auto MaxSweepDistance{ 2000.0f };

	if (InAirState.VerticalVelocity > VerticalVelocityThreshold)
	{
		InAirState.GroundPredictionAmount = 0.0f;
		return;
	}

	const auto AllowanceAmount{ 1.0f - GetCurveValueClamped01(UCharacterAnimCurveNames::GroundPredictionBlockCurveName()) };

	if (AllowanceAmount <= UE_KINDA_SMALL_NUMBER)
	{
		InAirState.GroundPredictionAmount = 0.0f;
		return;
	}

	const auto SweepStartLocation{ LocomotionState.Location };

	auto VelocityDirection{ LocomotionState.Velocity };

	VelocityDirection.Z = FMath::Clamp(VelocityDirection.Z, MinVerticalVelocity, MaxVerticalVelocity);
	VelocityDirection.Normalize();

	const auto SweepVector{ VelocityDirection * FMath::GetMappedRangeValueClamped(FVector2f(MaxVerticalVelocity, MinVerticalVelocity), FVector2f(MinSweepDistance, MaxSweepDistance), InAirState.VerticalVelocity) * LocomotionState.Scale };

	FHitResult Hit;
	GetWorld()->SweepSingleByChannel(
		Hit, 
		SweepStartLocation, 
		SweepStartLocation + SweepVector, 
		FQuat::Identity, 
		ECC_WorldStatic,
		FCollisionShape::MakeCapsule(LocomotionState.CapsuleRadius, LocomotionState.CapsuleHalfHeight),
		FCollisionQueryParams(__FUNCTION__, false, Character), 
		AnimData->InAir.GroundPredictionSweepResponses);
	
	const auto bGroundValid{ Hit.IsValidBlockingHit() && (Hit.ImpactNormal.Z >= LocomotionState.WalkableFloorZ) };

	InAirState.GroundPredictionAmount = bGroundValid ? (AnimData->InAir.GroundPredictionAmountCurve->GetFloatValue(Hit.Time) * AllowanceAmount) : 0.0f;
}

void UCharacterAnimInstance::UpdateInAirLeanAmount(float DeltaTime)
{
	// Use the direction and amount of relative velocity to determine how much the character will tilt

	static constexpr auto ReferenceSpeed{ 350.0f };

	const auto RelativeVelocity{ FVector3f(LocomotionState.RotationQuaternion.UnrotateVector(LocomotionState.Velocity)) /
								 ReferenceSpeed * AnimData->InAir.LeanAmountCurve->GetFloatValue(InAirState.VerticalVelocity)
	};

	if (bPendingUpdate)
	{
		LeanState.RightAmount = RelativeVelocity.Y;
		LeanState.ForwardAmount = RelativeVelocity.X;
	}
	else
	{
		LeanState.RightAmount = FMath::FInterpTo(LeanState.RightAmount, RelativeVelocity.Y,
			DeltaTime, AnimData->General.LeanInterpolationSpeed);

		LeanState.ForwardAmount = FMath::FInterpTo(LeanState.ForwardAmount, RelativeVelocity.X,
			DeltaTime, AnimData->General.LeanInterpolationSpeed);
	}
}

#pragma endregion


#pragma region Feet

void UCharacterAnimInstance::UpdateFeetOnGameThread()
{
	check(IsInGameThread());

	const auto* Mesh{ GetSkelMeshComponent() };

	const auto FootLeftTargetTransform{
		Mesh->GetSocketTransform(AnimData->General.bUseFootIkBones
									 ? UCharacterAnimCurveNames::FootLeftIkBoneName()
									 : UCharacterAnimCurveNames::FootLeftVirtualBoneName())
	};

	FeetState.Left.TargetLocation = FootLeftTargetTransform.GetLocation();
	FeetState.Left.TargetRotation = FootLeftTargetTransform.GetRotation();

	const auto FootRightTargetTransform{
		Mesh->GetSocketTransform(AnimData->General.bUseFootIkBones
									 ? UCharacterAnimCurveNames::FootRightIkBoneName()
									 : UCharacterAnimCurveNames::FootRightVirtualBoneName())
	};

	FeetState.Right.TargetLocation = FootRightTargetTransform.GetLocation();
	FeetState.Right.TargetRotation = FootRightTargetTransform.GetRotation();
}

void UCharacterAnimInstance::UpdateFeet(float DeltaTime)
{
	FeetState.FootPlantedAmount = FMath::Clamp(GetCurveValue(UCharacterAnimCurveNames::FootPlantedCurveName()), -1.0f, 1.0f);
	FeetState.FeetCrossingAmount = GetCurveValueClamped01(UCharacterAnimCurveNames::FeetCrossingCurveName());

	FeetState.MinMaxPelvisOffsetZ = FVector2D::ZeroVector;

	const auto ComponentTransformInverse{ GetProxyOnAnyThread<FAnimInstanceProxy>().GetComponentTransform().Inverse() };

	UpdateFoot(FeetState.Left, UCharacterAnimCurveNames::FootLeftIkCurveName(),
								UCharacterAnimCurveNames::FootLeftLockCurveName(), ComponentTransformInverse, DeltaTime);

	UpdateFoot(FeetState.Right, UCharacterAnimCurveNames::FootRightIkCurveName(),
								 UCharacterAnimCurveNames::FootRightLockCurveName(), ComponentTransformInverse, DeltaTime);

	FeetState.MinMaxPelvisOffsetZ.X = FMath::Min(FeetState.Left.OffsetTargetLocation.Z, FeetState.Right.OffsetTargetLocation.Z) /
												 LocomotionState.Scale;

	FeetState.MinMaxPelvisOffsetZ.Y = FMath::Max(FeetState.Left.OffsetTargetLocation.Z, FeetState.Right.OffsetTargetLocation.Z) /
												 LocomotionState.Scale;
}

void UCharacterAnimInstance::UpdateFoot(FFootState& FootState, const FName& FootIkCurveName, const FName& FootLockCurveName, const FTransform& ComponentTransformInverse, float DeltaTime) const
{
	FootState.IkAmount = GetCurveValueClamped01(FootIkCurveName);

	ProcessFootLockTeleport(FootState);

	ProcessFootLockBaseChange(FootState, ComponentTransformInverse);

	auto FinalLocation{ FootState.TargetLocation };
	auto FinalRotation{ FootState.TargetRotation };

	UpdateFootLock(FootState, FootLockCurveName, ComponentTransformInverse, DeltaTime, FinalLocation, FinalRotation);

	UpdateFootOffset(FootState, DeltaTime, FinalLocation, FinalRotation);

	FootState.IkLocation = ComponentTransformInverse.TransformPosition(FinalLocation);
	FootState.IkRotation = ComponentTransformInverse.TransformRotation(FinalRotation);
}

void UCharacterAnimInstance::ProcessFootLockTeleport(FFootState& FootState) const
{
	// Assume that teleportation occurs in a short period of time due to network smoothing.

	if (bPendingUpdate || GetWorld()->TimeSince(TeleportedTime) > 0.2f ||
		!FAnimWeight::IsRelevant(FootState.IkAmount * FootState.LockAmount))
	{
		return;
	}

	const auto& ComponentTransform{ GetProxyOnAnyThread<FAnimInstanceProxy>().GetComponentTransform() };

	FootState.LockLocation = ComponentTransform.TransformPosition(FootState.LockComponentRelativeLocation);
	FootState.LockRotation = ComponentTransform.TransformRotation(FootState.LockComponentRelativeRotation);

	if (MovementBase.bHasRelativeLocation)
	{
		const auto BaseRotationInverse{ MovementBase.Rotation.Inverse() };

		FootState.LockMovementBaseRelativeLocation = BaseRotationInverse.RotateVector(FootState.LockLocation - MovementBase.Location);
		FootState.LockMovementBaseRelativeRotation = BaseRotationInverse * FootState.LockRotation;
	}
}

void UCharacterAnimInstance::ProcessFootLockBaseChange(FFootState& FootState, const FTransform& ComponentTransformInverse) const
{
	if ((!bPendingUpdate && !MovementBase.bBaseChanged) || !FAnimWeight::IsRelevant(FootState.IkAmount * FootState.LockAmount))
	{
		return;
	}

	if (bPendingUpdate)
	{
		FootState.LockLocation = FootState.TargetLocation;
		FootState.LockRotation = FootState.TargetRotation;
	}

	FootState.LockComponentRelativeLocation = ComponentTransformInverse.TransformPosition(FootState.LockLocation);
	FootState.LockComponentRelativeRotation = ComponentTransformInverse.TransformRotation(FootState.LockRotation);

	if (MovementBase.bHasRelativeLocation)
	{
		const auto BaseRotationInverse{ MovementBase.Rotation.Inverse() };

		FootState.LockMovementBaseRelativeLocation = BaseRotationInverse.RotateVector(FootState.LockLocation - MovementBase.Location);
		FootState.LockMovementBaseRelativeRotation = BaseRotationInverse * FootState.LockRotation;
	}
	else
	{
		FootState.LockMovementBaseRelativeLocation = FVector::ZeroVector;
		FootState.LockMovementBaseRelativeRotation = FQuat::Identity;
	}
}

void UCharacterAnimInstance::UpdateFootLock(FFootState& FootState, const FName& FootLockCurveName, const FTransform& ComponentTransformInverse, float DeltaTime, FVector& FinalLocation, FQuat& FinalRotation) const
{
	auto NewFootLockAmount{ GetCurveValueClamped01(FootLockCurveName) };

	NewFootLockAmount *= 1.0f - RotateInPlaceState.FootLockBlockAmount;

	if (LocomotionState.bMovingSmooth || LocomotionMode != TAG_Status_LocomotionMode_OnGround)
	{
		// Smoothly disables leg locks when the character is moving or in the air.

		static constexpr auto MovingDecreaseSpeed{ 5.0f };
		static constexpr auto NotGroundedDecreaseSpeed{ 0.6f };

		NewFootLockAmount = bPendingUpdate
			? 0.0f
			: FMath::Max(
				0.0f, 
				FMath::Min(
					NewFootLockAmount,
					FootState.LockAmount - DeltaTime * (LocomotionState.bMovingSmooth ? MovingDecreaseSpeed : NotGroundedDecreaseSpeed)
				)
			);
	}

	if (AnimData->Feet.bDisableFootLock || !FAnimWeight::IsRelevant(FootState.IkAmount * NewFootLockAmount))
	{
		if (FootState.LockAmount > 0.0f)
		{
			FootState.LockAmount = 0.0f;

			FootState.LockLocation = FVector::ZeroVector;
			FootState.LockRotation = FQuat::Identity;

			FootState.LockComponentRelativeLocation = FVector::ZeroVector;
			FootState.LockComponentRelativeRotation = FQuat::Identity;

			FootState.LockMovementBaseRelativeLocation = FVector::ZeroVector;
			FootState.LockMovementBaseRelativeRotation = FQuat::Identity;
		}

		return;
	}

	const auto bNewAmountEqualOne{ FAnimWeight::IsFullWeight(NewFootLockAmount) };
	const auto bNewAmountGreaterThanPrevious{ NewFootLockAmount > FootState.LockAmount };

	// Update the footlocker amount only if the new amount is less than or equal to 1 of the current amount.

	if (bNewAmountEqualOne)
	{
		if (bNewAmountGreaterThanPrevious)
		{
			if (FootState.LockAmount <= 0.9f)
			{
				// Maintains the same locking position and rotation as the last time it was locked.

				FootState.LockLocation = FinalLocation;
				FootState.LockRotation = FinalRotation;
			}

			if (MovementBase.bHasRelativeLocation)
			{
				const auto BaseRotationInverse{ MovementBase.Rotation.Inverse() };

				FootState.LockMovementBaseRelativeLocation = BaseRotationInverse.RotateVector(FinalLocation - MovementBase.Location);
				FootState.LockMovementBaseRelativeRotation = BaseRotationInverse * FinalRotation;
			}
			else
			{
				FootState.LockMovementBaseRelativeLocation = FVector::ZeroVector;
				FootState.LockMovementBaseRelativeRotation = FQuat::Identity;
			}
		}

		FootState.LockAmount = 1.0f;
	}
	else if (!bNewAmountGreaterThanPrevious)
	{
		FootState.LockAmount = NewFootLockAmount;
	}

	if (MovementBase.bHasRelativeLocation)
	{
		FootState.LockLocation = MovementBase.Location + MovementBase.Rotation.RotateVector(FootState.LockMovementBaseRelativeLocation);
		FootState.LockRotation = MovementBase.Rotation * FootState.LockMovementBaseRelativeRotation;
	}

	FootState.LockComponentRelativeLocation = ComponentTransformInverse.TransformPosition(FootState.LockLocation);
	FootState.LockComponentRelativeRotation = ComponentTransformInverse.TransformRotation(FootState.LockRotation);

	FinalLocation = FMath::Lerp(FinalLocation, FootState.LockLocation, FootState.LockAmount);
	FinalRotation = FQuat::Slerp(FinalRotation, FootState.LockRotation, FootState.LockAmount);
}

void UCharacterAnimInstance::UpdateFootOffset(FFootState& FootState, float DeltaTime, FVector& FinalLocation, FQuat& FinalRotation) const
{
	if (!FAnimWeight::IsRelevant(FootState.IkAmount))
	{
		FootState.OffsetTargetLocation = FVector::ZeroVector;
		FootState.OffsetTargetRotation = FQuat::Identity;
		FootState.OffsetSpringState.Reset();
		return;
	}

	if (LocomotionMode == TAG_Status_LocomotionMode_InAir)
	{
		FootState.OffsetTargetLocation = FVector::ZeroVector;
		FootState.OffsetTargetRotation = FQuat::Identity;
		FootState.OffsetSpringState.Reset();

		if (bPendingUpdate)
		{
			FootState.OffsetLocation = FVector::ZeroVector;
			FootState.OffsetRotation = FQuat::Identity;
		}
		else
		{
			static constexpr auto InterpolationSpeed{ 15.0f };

			FootState.OffsetLocation = FMath::VInterpTo(FootState.OffsetLocation, FVector::ZeroVector, DeltaTime, InterpolationSpeed);
			FootState.OffsetRotation = FMath::QInterpTo(FootState.OffsetRotation, FQuat::Identity, DeltaTime, InterpolationSpeed);

			FinalLocation += FootState.OffsetLocation;
			FinalRotation = FootState.OffsetRotation * FinalRotation;
		}

		return;
	}

	// Trace down from the foot location to find the geometry. If the surface is walkable, save the impact location and normals

	const FVector TraceLocation{ FinalLocation.X, FinalLocation.Y, GetProxyOnAnyThread<FAnimInstanceProxy>().GetComponentTransform().GetLocation().Z };

	FHitResult Hit;
	GetWorld()->LineTraceSingleByChannel(
		Hit,
		TraceLocation + FVector(0.0f, 0.0f, AnimData->Feet.IkTraceDistanceUpward* LocomotionState.Scale),
		TraceLocation - FVector(0.0f, 0.0f, AnimData->Feet.IkTraceDistanceDownward * LocomotionState.Scale),
		UEngineTypes::ConvertToCollisionChannel(AnimData->Feet.IkTraceChannel),
		FCollisionQueryParams(__FUNCTION__, true, Character));

	const auto bGroundValid{ Hit.IsValidBlockingHit() && Hit.ImpactNormal.Z >= LocomotionState.WalkableFloorZ };

	if (bGroundValid)
	{
		const auto FootHeight{ AnimData->Feet.FootHeight * LocomotionState.Scale };

		// Find the difference in position between the impact location and the expected (flat) floor location.

		FootState.OffsetTargetLocation = Hit.ImpactPoint - TraceLocation + Hit.ImpactNormal * FootHeight;
		FootState.OffsetTargetLocation.Z -= FootHeight;

		// Calculate rotational offset

		FootState.OffsetTargetRotation = FRotator(
			-UMovementMathLibrary::DirectionToAngle(FVector2D(Hit.ImpactNormal.Z, Hit.ImpactNormal.X)),
			0.0f,
			UMovementMathLibrary::DirectionToAngle(FVector2D(Hit.ImpactNormal.Z, Hit.ImpactNormal.Y))).Quaternion();
	}

	// Interpolate current offset to new target value

	if (bPendingUpdate)
	{
		FootState.OffsetSpringState.Reset();

		FootState.OffsetLocation = FootState.OffsetTargetLocation;
		FootState.OffsetRotation = FootState.OffsetTargetRotation;
	}
	else
	{
		static constexpr auto LocationInterpolationFrequency{ 0.4f };
		static constexpr auto LocationInterpolationDampingRatio{ 4.0f };
		static constexpr auto LocationInterpolationTargetVelocityAmount{ 1.0f };

		FootState.OffsetLocation = UMovementMathLibrary::SpringDampVector(FootState.OffsetLocation, FootState.OffsetTargetLocation,
			FootState.OffsetSpringState, DeltaTime, LocationInterpolationFrequency,
			LocationInterpolationDampingRatio, LocationInterpolationTargetVelocityAmount);

		static constexpr auto RotationInterpolationSpeed{ 30.0f };

		FootState.OffsetRotation = FMath::QInterpTo(FootState.OffsetRotation, FootState.OffsetTargetRotation,
			DeltaTime, RotationInterpolationSpeed);
	}

	FinalLocation += FootState.OffsetLocation;
	FinalRotation = FootState.OffsetRotation * FinalRotation;
}

#pragma endregion


#pragma region Transitions

void UCharacterAnimInstance::PlayQuickStopAnimation()
{
	if (RotationMode != TAG_Status_RotationMode_VelocityDirection)
	{
		PlayTransitionLeftAnimation(AnimData->Transitions.QuickStopBlendInDuration, AnimData->Transitions.QuickStopBlendOutDuration,
			AnimData->Transitions.QuickStopPlayRate.X, AnimData->Transitions.QuickStopStartTime);
		return;
	}

	auto RotationYawAngle{ FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT((LocomotionState.bHasInput ? LocomotionState.InputYawAngle : LocomotionState.TargetYawAngle) - LocomotionState.Rotation.Yaw)) };

	if (RotationYawAngle > 180.0f - UMovementMathLibrary::CounterClockwiseRotationAngleThreshold)
	{
		RotationYawAngle -= 360.0f;
	}

	// Adjust the playback speed of the quick stop animation based on the distance of the character

	if (RotationYawAngle <= 0.0f)
	{
		PlayTransitionLeftAnimation(AnimData->Transitions.QuickStopBlendInDuration, AnimData->Transitions.QuickStopBlendOutDuration,
			FMath::Lerp(AnimData->Transitions.QuickStopPlayRate.X, AnimData->Transitions.QuickStopPlayRate.Y,
				FMath::Abs(RotationYawAngle) / 180.0f), AnimData->Transitions.QuickStopStartTime);
	}
	else
	{
		PlayTransitionRightAnimation(AnimData->Transitions.QuickStopBlendInDuration, AnimData->Transitions.QuickStopBlendOutDuration,
			FMath::Lerp(AnimData->Transitions.QuickStopPlayRate.X, AnimData->Transitions.QuickStopPlayRate.Y,
				FMath::Abs(RotationYawAngle) / 180.0f), AnimData->Transitions.QuickStopStartTime);
	}
}

void UCharacterAnimInstance::PlayTransitionAnimation(UAnimSequenceBase* Animation, float BlendInDuration, float BlendOutDuration, float PlayRate, float StartTime, bool bFromStandingIdleOnly)
{
	check(IsInGameThread());

	if (!IsValid(CharacterMovement))
	{
		return;
	}

	if (bFromStandingIdleOnly && (CharacterMovement->GetLocomotionState().bMoving || CharacterMovement->GetStance() != TAG_Status_Stance_Standing))
	{
		return;
	}

	PlaySlotAnimationAsDynamicMontage(Animation, UCharacterAnimCurveNames::TransitionSlotName(), BlendInDuration, BlendOutDuration, PlayRate, 1, 0.0f, StartTime);
}

void UCharacterAnimInstance::PlayTransitionLeftAnimation(float BlendInDuration, float BlendOutDuration, float PlayRate, float StartTime, bool bFromStandingIdleOnly)
{
	if (!IsValid(AnimData))
	{
		return;
	}

	PlayTransitionAnimation(Stance == TAG_Status_Stance_Crouching
		? AnimData->Transitions.CrouchingTransitionLeftAnimation
		: AnimData->Transitions.StandingTransitionLeftAnimation,
		BlendInDuration, BlendOutDuration, PlayRate, StartTime, bFromStandingIdleOnly);
}

void UCharacterAnimInstance::PlayTransitionRightAnimation(float BlendInDuration, float BlendOutDuration, float PlayRate, float StartTime, bool bFromStandingIdleOnly)
{
	if (!IsValid(AnimData))
	{
		return;
	}

	PlayTransitionAnimation(Stance == TAG_Status_Stance_Crouching
		? AnimData->Transitions.CrouchingTransitionRightAnimation
		: AnimData->Transitions.StandingTransitionRightAnimation,
		BlendInDuration, BlendOutDuration, PlayRate, StartTime, bFromStandingIdleOnly);
}

void UCharacterAnimInstance::StopTransitionAndTurnInPlaceAnimations(float BlendOutDuration)
{
	check(IsInGameThread());

	StopSlotAnimation(BlendOutDuration, UCharacterAnimCurveNames::TransitionSlotName());
}

void UCharacterAnimInstance::UpdateTransitions()
{
	// Because the allowed transition curve changes within certain states, the allowed transitions are true in those states.

	TransitionsState.bTransitionsAllowed = FAnimWeight::IsFullWeight(GetCurveValue(UCharacterAnimCurveNames::AllowTransitionsCurveName()));

	UpdateDynamicTransition();
}

void UCharacterAnimInstance::UpdateDynamicTransition()
{
	if (TransitionsState.DynamicTransitionsFrameDelay > 0)
	{
		TransitionsState.DynamicTransitionsFrameDelay -= 1;
		return;
	}

	if (!TransitionsState.bTransitionsAllowed || LocomotionState.bMoving || LocomotionMode != TAG_Status_LocomotionMode_OnGround)
	{
		return;
	}

	// Check each foot to see the difference between the appearance of the foot and its position relative to its desired/target position.

	const auto FootLockDistanceThresholdSquared{ FMath::Square(AnimData->Transitions.DynamicTransitionFootLockDistanceThreshold * LocomotionState.Scale) };

	const auto FootLockLeftDistanceSquared{ FVector::DistSquared(FeetState.Left.TargetLocation, FeetState.Left.LockLocation) };
	const auto FootLockRightDistanceSquared{ FVector::DistSquared(FeetState.Right.TargetLocation, FeetState.Right.LockLocation) };

	const auto bTransitionLeftAllowed{ FAnimWeight::IsRelevant(FeetState.Left.LockAmount) && FootLockLeftDistanceSquared > FootLockDistanceThresholdSquared };
	const auto bTransitionRightAllowed{ FAnimWeight::IsRelevant(FeetState.Right.LockAmount) && FootLockRightDistanceSquared > FootLockDistanceThresholdSquared };

	if (!bTransitionLeftAllowed && !bTransitionRightAllowed)
	{
		return;
	}

	TObjectPtr<UAnimSequenceBase> DynamicTransitionAnimation;

	// If both transitions are allowed, select the one with the greater locking distance.

	if (!bTransitionLeftAllowed)
	{
		DynamicTransitionAnimation = Stance == TAG_Status_Stance_Crouching
			? AnimData->Transitions.CrouchingDynamicTransitionRightAnimation
			: AnimData->Transitions.StandingDynamicTransitionRightAnimation;
	}
	else if (!bTransitionRightAllowed)
	{
		DynamicTransitionAnimation = Stance == TAG_Status_Stance_Crouching
			? AnimData->Transitions.CrouchingDynamicTransitionLeftAnimation
			: AnimData->Transitions.StandingDynamicTransitionLeftAnimation;
	}
	else if (FootLockLeftDistanceSquared >= FootLockRightDistanceSquared)
	{
		DynamicTransitionAnimation = Stance == TAG_Status_Stance_Crouching
			? AnimData->Transitions.CrouchingDynamicTransitionLeftAnimation
			: AnimData->Transitions.StandingDynamicTransitionLeftAnimation;
	}
	else
	{
		DynamicTransitionAnimation = Stance == TAG_Status_Stance_Crouching
			? AnimData->Transitions.CrouchingDynamicTransitionRightAnimation
			: AnimData->Transitions.StandingDynamicTransitionRightAnimation;
	}

	if (IsValid(DynamicTransitionAnimation))
	{
		// Block the next dynamic transition by approximately two frames to give the animation blueprint time to react properly to the animation.

		TransitionsState.DynamicTransitionsFrameDelay = 2;

		// Animated montages cannot be played in the worker thread, so they are queued and played later in the game thread.

		TransitionsState.QueuedDynamicTransitionAnimation = DynamicTransitionAnimation;

		if (IsInGameThread())
		{
			PlayQueuedDynamicTransitionAnimation();
		}
	}
}

void UCharacterAnimInstance::PlayQueuedDynamicTransitionAnimation()
{
	check(IsInGameThread());

	PlaySlotAnimationAsDynamicMontage(TransitionsState.QueuedDynamicTransitionAnimation, UCharacterAnimCurveNames::TransitionSlotName(),
		AnimData->Transitions.DynamicTransitionBlendDuration,
		AnimData->Transitions.DynamicTransitionBlendDuration,
		AnimData->Transitions.DynamicTransitionPlayRate, 1, 0.0f);

	TransitionsState.QueuedDynamicTransitionAnimation = nullptr;
}

#pragma endregion


#pragma region Rotate In Place

bool UCharacterAnimInstance::IsRotateInPlaceAllowed()
{
	return RotationMode != TAG_Status_RotationMode_VelocityDirection;
}

void UCharacterAnimInstance::UpdateRotateInPlace(float DeltaTime)
{
	static constexpr auto PlayRateInterpolationSpeed{ 5.0f };

	// Rotation in place is only permitted when the character is stationary and aiming, or in first-person view mode.

	if (LocomotionState.bMoving || LocomotionMode != TAG_Status_LocomotionMode_OnGround || !IsRotateInPlaceAllowed())
	{
		RotateInPlaceState.bRotatingLeft = false;
		RotateInPlaceState.bRotatingRight = false;

		RotateInPlaceState.PlayRate = bPendingUpdate
			? AnimData->RotateInPlace.PlayRate.X
			: FMath::FInterpTo(RotateInPlaceState.PlayRate, AnimData->RotateInPlace.PlayRate.X,
				DeltaTime, PlayRateInterpolationSpeed);

		RotateInPlaceState.FootLockBlockAmount = 0.0f;
		return;
	}

	// Check if the yaw angle of the view exceeds the threshold to see if the character should be rotated left or right.

	RotateInPlaceState.bRotatingLeft = ViewState.YawAngle < -AnimData->RotateInPlace.ViewYawAngleThreshold;
	RotateInPlaceState.bRotatingRight = ViewState.YawAngle > AnimData->RotateInPlace.ViewYawAngleThreshold;

	if (!RotateInPlaceState.bRotatingLeft && !RotateInPlaceState.bRotatingRight)
	{
		RotateInPlaceState.PlayRate = bPendingUpdate
			? AnimData->RotateInPlace.PlayRate.X
			: FMath::FInterpTo(RotateInPlaceState.PlayRate, AnimData->RotateInPlace.PlayRate.X,
				DeltaTime, PlayRateInterpolationSpeed);

		RotateInPlaceState.FootLockBlockAmount = 0.0f;
		return;
	}

	// If the character needs to be rotated, set the playback rate to match the yaw of the view.

	const auto PlayRate{ FMath::GetMappedRangeValueClamped(AnimData->RotateInPlace.ReferenceViewYawSpeed, AnimData->RotateInPlace.PlayRate, ViewState.YawSpeed) };

	RotateInPlaceState.PlayRate = bPendingUpdate
		? PlayRate
		: FMath::FInterpTo(RotateInPlaceState.PlayRate, PlayRate,
			DeltaTime, PlayRateInterpolationSpeed);

	// Disable the foot lock when rotating at large angles or rotating too fast. Otherwise, the legs may twist spirally.

	static constexpr auto BlockInterpolationSpeed{ 5.0f };

	RotateInPlaceState.FootLockBlockAmount =
		AnimData->RotateInPlace.bDisableFootLock
		? 1.0f
		: FMath::Abs(ViewState.YawAngle) > AnimData->RotateInPlace.FootLockBlockViewYawAngleThreshold
		? 0.5f
		: ViewState.YawSpeed <= AnimData->RotateInPlace.FootLockBlockViewYawSpeedThreshold
		? 0.0f
		: bPendingUpdate
		? 1.0f
		: FMath::FInterpTo(RotateInPlaceState.FootLockBlockAmount, 1.0f, DeltaTime, BlockInterpolationSpeed);
}

#pragma endregion


#pragma region Utilities

float UCharacterAnimInstance::GetCurveValueClamped01(const FName& CurveName) const
{
	return UMovementMathLibrary::Clamp01(GetCurveValue(CurveName));
}

#pragma endregion

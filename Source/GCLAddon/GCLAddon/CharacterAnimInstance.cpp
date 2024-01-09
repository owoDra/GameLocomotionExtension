// Copyright (C) 2024 owoDra

#include "CharacterAnimInstance.h"

#include "GameplayTag/GCLATags_Status.h"
#include "LocomotionGeneralNameStatics.h"
#include "LocomotionFunctionLibrary.h"
#include "LocomotionComponent.h"
#include "LocomotionCharacter.h"
#include "GCLAddonStatGroup.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstanceProxy.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterAnimInstance)


UCharacterAnimInstance::UCharacterAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
}


#pragma region Initialization

void UCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<ALocomotionCharacter>(GetOwningActor());

#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld() && !IsValid(Character))
	{
		// Use CDO of Character for display in editor

		Character = GetMutableDefault<ALocomotionCharacter>();
	}
#endif

	CharacterMovement = Cast<ULocomotionComponent>(Character->GetCharacterMovement());
}

void UCharacterAnimInstance::NativeBeginPlay()
{
	ensureMsgf(IsValid(Character)			, TEXT("UCharacterAnimInstance::NativeBeginPlay: Invalid Character"));
	ensureMsgf(IsValid(CharacterMovement)	, TEXT("UCharacterAnimInstance::NativeBeginPlay: Invalid CharacterMovement"));
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UCharacterAnimInstance::NativeUpdateAnimation()"), STAT_UCharacterAnimInstance_NativeUpdateAnimation, STATGROUP_Locomotion);

	Super::NativeUpdateAnimation(DeltaTime);

	UpdateAnimationOnGameThread(DeltaTime);
}

void UCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UCharacterAnimInstance::NativeThreadSafeUpdateAnimation()"), STAT_UCharacterAnimInstance_NativeThreadSafeUpdateAnimation, STATGROUP_Locomotion);

	Super::NativeThreadSafeUpdateAnimation(DeltaTime);

	UpdateAnimationOnThreadSafe(DeltaTime);
}

void UCharacterAnimInstance::NativePostEvaluateAnimation()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UCharacterAnimInstance::NativePostEvaluateAnimation()"), STAT_UCharacterAnimInstance_NativePostEvaluateAnimation, STATGROUP_Locomotion)

	Super::NativePostEvaluateAnimation();

	OnPostEvaluateAnimation();
}

void UCharacterAnimInstance::UpdateAnimationOnGameThread(float DeltaTime)
{
	if (!IsValid(Character) || !IsValid(CharacterMovement))
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

		const_cast<FTransform&>(Proxy.GetComponentTransform()) = GetSkelMeshComponent()->GetComponentTransform();
		const_cast<FTransform&>(Proxy.GetComponentRelativeTransform()) = GetSkelMeshComponent()->GetRelativeTransform();
		const_cast<FTransform&>(Proxy.GetActorTransform()) = ActorTransform;
	}

	LocomotionMode = CharacterMovement->GetLocomotionMode();
	RotationMode = CharacterMovement->GetRotationMode();
	Stance = CharacterMovement->GetStance();
	Gait = CharacterMovement->GetGait();
	LocomotionAction = CharacterMovement->GetLocomotionAction();

	UpdateCharacterStatesOnGameThread();
	UpdateMovementBaseOnGameThread();
	UpdateViewOnGameThread();
	UpdateLocomotionOnGameThread();
}

void UCharacterAnimInstance::UpdateAnimationOnThreadSafe(float DeltaTime)
{
	if (!IsValid(Character) || !IsValid(CharacterMovement))
	{
		return;
	}

	UpdateView(DeltaTime);
}

void UCharacterAnimInstance::OnPostEvaluateAnimation()
{
	if (!IsValid(Character) || !IsValid(CharacterMovement))
	{
		return;
	}

	bPendingUpdate = false;
}

#pragma endregion


#pragma region Character States

void UCharacterAnimInstance::UpdateCharacterStatesOnGameThread()
{
	LocomotionMode = CharacterMovement->GetLocomotionMode();
	RotationMode = CharacterMovement->GetRotationMode();
	Stance = CharacterMovement->GetStance();
	Gait = CharacterMovement->GetGait();
	LocomotionAction = CharacterMovement->GetLocomotionAction();
}

#pragma endregion


#pragma region Movement Base

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

#pragma endregion


#pragma region View State

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
		ViewState.YawAngle = FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw - LocomotionState.Rotation.Yaw));
		ViewState.PitchAngle = FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Pitch - LocomotionState.Rotation.Pitch));

		ViewState.PitchAmount = (0.5f - ViewState.PitchAngle / 180.0f);
	}

	ViewState.ViewAmount = 1.0f - GetCurveValueClamped01(ULocomotionGeneralNameStatics::ViewBlockCurveName());
	ViewState.AimingAmount = GetCurveValueClamped01(ULocomotionGeneralNameStatics::AllowAimingCurveName());
	ViewState.LookAmount = (ViewState.ViewAmount * (1.0f - ViewState.AimingAmount));
}

#pragma endregion


#pragma region Locomotion State

void UCharacterAnimInstance::UpdateLocomotionOnGameThread()
{
	check(IsInGameThread());

	const auto& Locomotion{ CharacterMovement->GetLocomotionState() };

	LocomotionState.bHasInput = Locomotion.bHasInput;
	LocomotionState.InputYawAngle = Locomotion.InputYawAngle;

	LocomotionState.Speed = Locomotion.Speed;
	LocomotionState.Velocity = Locomotion.Velocity;
	LocomotionState.VelocityYawAngle = Locomotion.VelocityYawAngle;
	LocomotionState.Acceleration = Locomotion.Acceleration;

	LocomotionState.MaxAcceleration = CharacterMovement->GetMaxAcceleration();
	LocomotionState.MaxBrakingDeceleration = CharacterMovement->GetMaxBrakingDeceleration();
	LocomotionState.WalkableFloorZ = CharacterMovement->GetWalkableFloorZ();

	LocomotionState.bMoving = Locomotion.bMoving;

	LocomotionState.bMovingSmooth = (Locomotion.bHasInput && Locomotion.bHasSpeed) || (Locomotion.Speed > MovingSmoothSpeedThreshold);

	LocomotionState.TargetYawAngle = Locomotion.TargetYawAngle;
	LocomotionState.Location = Locomotion.Location;
	LocomotionState.Rotation = Locomotion.Rotation;
	LocomotionState.RotationQuaternion = Locomotion.RotationQuaternion;
	LocomotionState.YawSpeed = Locomotion.YawSpeed;

	LocomotionState.Scale = UE_REAL_TO_FLOAT(GetSkelMeshComponent()->GetComponentScale().Z);

	const auto* Capsule{ Character->GetCapsuleComponent() };

	LocomotionState.CapsuleRadius = Capsule->GetScaledCapsuleRadius();
	LocomotionState.CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
}

#pragma endregion


#pragma region Utilities

float UCharacterAnimInstance::GetCurveValueClamped01(const FName& CurveName) const
{
	return ULocomotionFunctionLibrary::Clamp01(GetCurveValue(CurveName));
}

#pragma endregion

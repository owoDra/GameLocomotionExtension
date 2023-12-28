// Copyright (C) 2023 owoDra

#pragma once

#include "Animation/AnimInstance.h"

#include "Movement/State/ControlRigInput.h"
#include "Movement/State/FeetState.h"
#include "Movement/State/GroundedState.h"
#include "Movement/State/InAirState.h"
#include "Movement/State/LayeringState.h"
#include "Movement/State/LeanState.h"
#include "Movement/State/LocomotionAnimationState.h"
#include "Movement/State/MovementBaseState.h"
#include "Movement/State/PoseState.h"
#include "Movement/State/RotateInPlaceState.h"
#include "Movement/State/TransitionsState.h"
#include "Movement/State/ViewAnimationState.h"
#include "Movement/State/ViewState.h"
#include "Movement/State/LocomotionState.h"
#include "Movement/State/MovementBaseState.h"

#include "GameplayTagContainer.h"

#include "CharacterAnimInstance.generated.h"

class UCharacterAnimData;
class UCharacterLinkedAnimInstance;
class UGCLACharacterMovementComponent;
class AGCLACharacter;
enum class EHipsDirection : uint8;


/**
 * Main AnimInsntace class specialized for character features
 * 
 * Tips:
 *	Basically, it is used only for TPP Mesh of Character and processes data necessary for animation.
 */
UCLASS(Config = Game)
class GCLADDON_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	friend UCharacterLinkedAnimInstance;

public:
	UCharacterAnimInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//
	// Data assets that define settings for character animation
	//
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Configs")
	TObjectPtr<UCharacterAnimData> AnimData;

	//
	// Owner Character of the Mesh that owns this AnimInstance.
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", Transient)
	TObjectPtr<AGCLACharacter> Character;

	//
	// Character movement component of OwnerCharacter
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", Transient)
	TObjectPtr<UGCLACharacterMovementComponent> CharacterMovement;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag LocomotionMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag RotationMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag Stance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag LocomotionAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FMovementBaseState MovementBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FLayeringState LayeringState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FPoseState PoseState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FViewAnimationState ViewState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FLeanState LeanState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FLocomotionAnimationState LocomotionState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGroundedState GroundedState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FInAirState InAirState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FFeetState FeetState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FTransitionsState TransitionsState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FRotateInPlaceState RotateInPlaceState;

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaTime) override;
	virtual void NativePostEvaluateAnimation() override;

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;


protected:
	//
	// Flag indicating that the animation instance has not been updated for a long time and its current state may be incorrect.
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	bool bPendingUpdate{ true };

	//
	// Time the last teleportation took place.
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Meta = (ClampMin = 0))
	float TeleportedTime;

protected:
	/**
	 * Getting AnimData with BlueprintThreadSafe
	 */
	UFUNCTION(BlueprintPure, Category = "Character Anim Instance", Meta = (BlueprintProtected, BlueprintThreadSafe, ReturnDisplayName = "Setting"))
	UCharacterAnimData* GetAnimDataUnsafe() const { return AnimData; }

public:
	/**
	 * Get data to pass to ControlRig in BlueprintThreadSafe
	 */
	UFUNCTION(BlueprintPure, Category = "Character Anim Instance", Meta = (BlueprintThreadSafe, ReturnDisplayName = "Rig Input"))
	FControlRigInput GetControlRigInput() const;

public:
	void MarkPendingUpdate() { bPendingUpdate |= true; }

	void MarkTeleported();

private:
	void UpdateMovementBaseOnGameThread();

	void UpdateLayering();

	void UpdatePose();


	/////////////////////////////////////////
	// View
public:
	virtual bool IsSpineRotationAllowed();

private:
	void UpdateViewOnGameThread();

	void UpdateView(float DeltaTime);

	void UpdateSpineRotation(float DeltaTime);

protected:
	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ReinitializeLook();

	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void UpdateLook();


	/////////////////////////////////////////
	// Locomotion
private:
	void UpdateLocomotionOnGameThread();


	/////////////////////////////////////////
	// OnGround
protected:
	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void SetHipsDirection(EHipsDirection NewHipsDirection);

	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ActivatePivot();

private:
	void UpdateGroundedOnGameThread();

	void UpdateGrounded(float DeltaTime);

	void UpdateMovementDirection();

	void UpdateVelocityBlend(float DeltaTime);

	void UpdateRotationYawOffsets();

	void UpdateSprint(const FVector3f& RelativeAccelerationAmount, float DeltaTime);

	void UpdateStrideBlendAmount();

	void UpdateWalkRunBlendAmount();

	void UpdateStandingPlayRate();

	void UpdateCrouchingPlayRate();

	void UpdateGroundedLeanAmount(const FVector3f& RelativeAccelerationAmount, float DeltaTime);

	void ResetGroundedLeanAmount(float DeltaTime);


	/////////////////////////////////////////
	// InAir
protected:
	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ResetJumped();

private:
	void UpdateInAirOnGameThread();

	void UpdateInAir(float DeltaTime);

	void UpdateGroundPredictionAmount();

	void UpdateInAirLeanAmount(float DeltaTime);


	/////////////////////////////////////////
	// Feet
private:
	void UpdateFeetOnGameThread();

	void UpdateFeet(float DeltaTime);

	void UpdateFoot(FFootState& FootState, const FName& FootIkCurveName, const FName& FootLockCurveName, const FTransform& ComponentTransformInverse, float DeltaTime) const;

	void ProcessFootLockTeleport(FFootState& FootState) const;

	void ProcessFootLockBaseChange(FFootState& FootState, const FTransform& ComponentTransformInverse) const;

	void UpdateFootLock(FFootState& FootState, const FName& FootLockCurveName, const FTransform& ComponentTransformInverse, float DeltaTime, FVector& FinalLocation, FQuat& FinalRotation) const;

	void UpdateFootOffset(FFootState& FootState, float DeltaTime, FVector& FinalLocation, FQuat& FinalRotation) const;


	/////////////////////////////////////////
	// Transitions
public:
	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance")
	void PlayQuickStopAnimation();

	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance")
	void PlayTransitionAnimation(UAnimSequenceBase* Animation, float BlendInDuration = 0.2f, float BlendOutDuration = 0.2f, float PlayRate = 1.0f, float StartTime = 0.0f, bool bFromStandingIdleOnly = false);

	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance")
	void PlayTransitionLeftAnimation(float BlendInDuration = 0.2f, float BlendOutDuration = 0.2f, float PlayRate = 1.0f, float StartTime = 0.0f, bool bFromStandingIdleOnly = false);

	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance")
	void PlayTransitionRightAnimation(float BlendInDuration = 0.2f, float BlendOutDuration = 0.2f, float PlayRate = 1.0f, float StartTime = 0.0f, bool bFromStandingIdleOnly = false);

	UFUNCTION(BlueprintCallable, Category = "Character Anim Instance")
	void StopTransitionAndTurnInPlaceAnimations(float BlendOutDuration = 0.2f);

private:
	void UpdateTransitions();

	void UpdateDynamicTransition();

	void PlayQueuedDynamicTransitionAnimation();


	/////////////////////////////////////////
	// Rotate In Place
public:
	virtual bool IsRotateInPlaceAllowed();

private:
	void UpdateRotateInPlace(float DeltaTime);


	/////////////////////////////////////////
	// Utilities
public:
	float GetCurveValueClamped01(const FName& CurveName) const;

};

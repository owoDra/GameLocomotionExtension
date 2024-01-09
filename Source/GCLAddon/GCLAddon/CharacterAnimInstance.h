// Copyright (C) 2024 owoDra

#pragma once

#include "Animation/AnimInstance.h"

#include "State/ViewState.h"
#include "State/LocomotionState.h"
#include "State/MovementBaseState.h"
#include "State/AnimationViewState.h"
#include "State/AnimationLocomotionState.h"

#include "GameplayTagContainer.h"

#include "CharacterAnimInstance.generated.h"

class ULocomotionComponent;
class ALocomotionCharacter;


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
public:
	UCharacterAnimInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	//
	// Owner Character of the Mesh that owns this AnimInstance.
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", Transient)
	TObjectPtr<ALocomotionCharacter> Character;

	//
	// Character movement component of OwnerCharacter
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", Transient)
	TObjectPtr<ULocomotionComponent> CharacterMovement;


	/////////////////////////////////////////
	// Initialization
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaTime) override;
	virtual void NativePostEvaluateAnimation() override;

protected:
	virtual void UpdateAnimationOnGameThread(float DeltaTime);
	virtual void UpdateAnimationOnThreadSafe(float DeltaTime);
	virtual void OnPostEvaluateAnimation();


	/////////////////////////////////////////
	// Flag
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

public:
	void MarkPendingUpdate() { bPendingUpdate |= true; }

	void MarkTeleported() { TeleportedTime = GetWorld()->GetTimeSeconds(); }


	/////////////////////////////////////////
	// Character States
public:
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

protected:
	void UpdateCharacterStatesOnGameThread();


	/////////////////////////////////////////
	// Movement Base
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FMovementBaseState MovementBase;

protected:
	void UpdateMovementBaseOnGameThread();


	/////////////////////////////////////////
	// View State
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FAnimationViewState ViewState;

protected:
	void UpdateViewOnGameThread();

	void UpdateView(float DeltaTime);


	/////////////////////////////////////////
	// Locomotion State
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FAnimationLocomotionState LocomotionState;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Configs|General", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float MovingSmoothSpeedThreshold{ 150.0 };

protected:
	void UpdateLocomotionOnGameThread();


	/////////////////////////////////////////
	// Utilities
public:
	float GetCurveValueClamped01(const FName& CurveName) const;

};

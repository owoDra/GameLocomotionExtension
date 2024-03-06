// Copyright (C) 2024 owoDra

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"

#include "State/ViewState.h"
#include "State/MovementBaseState.h"
#include "State/LocomotionState.h"
#include "Type/LocomotionConfigTypes.h"
#include "Type/LocomotionNetworkTypes.h"

#include "GameplayTagContainer.h"

#include "LocomotionComponent.generated.h"

class ULocomotionData;
class UCustomMovementProcess;
struct FBasedMovementInfo;
struct FRuntimeFloatCurve;


/**
 * Components that extend the processing of character movement relationships
 */
UCLASS()
class GLEXT_API ULocomotionComponent
	: public UCharacterMovementComponent
	, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	friend class FLocomotionSavedMove;
	friend class UCustomMovementProcess;

public:
	ULocomotionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	//
	// Function name used to add this component
	// 
	static const FName NAME_ActorFeatureName;

protected:
	FLocomotionNetworkMoveDataContainer MoveDataContainer;

	////////////////////////////////////////////////
	// Initialize and Deinitialize
#pragma region Initialize and Deinitialize
protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;

protected:
	virtual bool CanChangeInitStateToSpawned(UGameFrameworkComponentManager* Manager) const { return true; }
	virtual bool CanChangeInitStateToDataAvailable(UGameFrameworkComponentManager* Manager) const { return true; }
	virtual bool CanChangeInitStateToDataInitialized(UGameFrameworkComponentManager* Manager) const { return true; }
	virtual bool CanChangeInitStateToGameplayReady(UGameFrameworkComponentManager* Manager) const { return true; }

	virtual void HandleChangeInitStateToSpawned(UGameFrameworkComponentManager* Manager) {}
	virtual void HandleChangeInitStateToDataAvailable(UGameFrameworkComponentManager* Manager) {}
	virtual void HandleChangeInitStateToDataInitialized(UGameFrameworkComponentManager* Manager) {}
	virtual void HandleChangeInitStateToGameplayReady(UGameFrameworkComponentManager* Manager) {}

#pragma endregion


	////////////////////////////////////////////////
	// Locomotion Data
#pragma region Locomotion Data

	/**
	 * @TODO: Consider whether to replicate LocomotionData.
	 *	Currently, LocomotionData must be initialized through events that are executed from both the client and server, 
	 *	or it must be set up at the CDO stage.
	 */

protected:
	//
	// Current locomotion data
	//
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TObjectPtr<const ULocomotionData> LocomotionData;

	//
	// Instance list of CustomMovementProcess defined by LocomotionData
	//
	UPROPERTY(Transient)
	TMap<uint8, TObjectPtr<UCustomMovementProcess>> CustomMovementProcesses;

protected:
	/**
	 * Apply the current locomotion data
	 */
	virtual void ApplyLocomotionData();

	/**
	 * Create instances of CustomMovementProcess defined in LocomotionData.
	 */
	void CreateCustomMovementProcesses();

	/**
	 * Calls when locomotion data set or changed
	 */
	virtual void HandleLocomotionDataUpdated();

public:
	/**
	 * Set the current locomotion data
	 * 
	 * Note:
	 *	This function must be executed on all clients and servers
	 */
	UFUNCTION(BlueprintCallable)
	void SetLocomotionData(const ULocomotionData* NewLocomotionData);

#pragma endregion


	////////////////////////////////////////////////
	// Locomotion Space
#pragma region Locomotion Space
protected:
	//
	// Current locomotion space
	//
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "State|LocomotionSpace")
	ELocomotionSpace LocomotionSpace{ ELocomotionSpace::None };

public:
	/**
	 * Set the current locomotion space
	 */
	void SetLocomotionSpace(ELocomotionSpace NewLocomotionSpace) { LocomotionSpace = NewLocomotionSpace; }

	/**
	 * Returns whether Locomotion Space is OnGround or not.
	 */
	virtual bool IsMovingOnGround() const override { return LocomotionSpace == ELocomotionSpace::OnGround; }

	/**
	 * Returns whether Locomotion Space is InAir or not.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Locomotion State")
	virtual bool IsMovingInAir() const { return LocomotionSpace == ELocomotionSpace::InAir; }

	/**
	 * Returns whether Locomotion Space is InWater or not.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Locomotion State")
	virtual bool IsMovingInWater() const { return LocomotionSpace == ELocomotionSpace::InWater; }

#pragma endregion


	////////////////////////////////////////////////
	// Movement Base
#pragma region Movement Base
protected:
	//
	// Information for the Character's movement base
	// 
	// Tips:
	//	For example, in Mantle, the object being surmounted becomes the base
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|MovementBase", Transient)
	FMovementBaseState MovementBase;

public:
	virtual void UpdateBasedRotation(FRotator& FinalRotation, const FRotator& ReducedRotation) override;

	/**
	 * Determine if absolute rotation should be used and, if necessary, update to use absolute rotation
	 *
	 * Tips:
	 *  Use absolute rotation of the Mesh so that character rotation can be precisely synchronized
	 *  Update animation by manually updating mesh rotation from AnimInstance
	 *
	 *  This is necessary if the Character and AnimInstance are updated at different frequencies
	 *	and helps synchronize the rotation of the Character and AnimInstance.
	 *
	 * Note:
	 *  Use only when really necessary to save performance
	 *  (For example, if URO is enabled, or if you are an autonomous proxy on a listening server)
	 */
	void UpdateUsingAbsoluteRotation() const;

	/**
	 * Update Visibility and Tick settings
	 */
	void UpdateVisibilityBasedAnimTickOption() const;

	/**
	 * Update MovementBase
	 */
	void UpdateMovementBase();

	/**
	 * Notify AnimInstance of updates
	 */
	void UpdateAnimInstanceMovement();

#pragma endregion


	//////////////////////////////////////////
	// Locomotion Mode
#pragma region Locomotion Mode
protected:
	//
	// Current locomotion mode of Character
	// (OnGround, InAir ...)
	// 
	// Tips:
	//	Updated accordingly when Movement Mode is changed
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Locomotion Mode", Transient)
	FGameplayTag LocomotionMode;

	//
	// Whether MovementMode changes are disabled or not
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Locomotion Mode", Transient)
	bool bMovementModeLocked;

public:
	virtual void SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode = 0) override;

	/**
	 * Return the current Locomotion Mode.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State|Locomotion Mode")
	const FGameplayTag& GetLocomotionMode() const { return LocomotionMode; }

protected:
	/**
	 * Set current bMovementModeLocked value
	 */
	void SetMovementModeLocked(bool bNewMovementModeLocked) { bMovementModeLocked = bNewMovementModeLocked; }

	/**
	 * Set current LocomotionMode
	 */
	void SetLocomotionMode(const FGameplayTag& NewLocomotionMode);

	/**
	 * Update each State by switching the referenced Configs according to the current DesiredXXX value.
	 */
	void UpdateLocomotionConfigs();

	/**
	 * Notify that MovementMode has changed
	 */
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

#pragma endregion


	////////////////////////////////////////////////
	// Desired Rotation Mode
#pragma region Desired Rotation Mode
protected:
	//
	// Rotation Mode of the Character for which the transition is desired.
	// (Velocity Direction, View Direction, Aiming ...)
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs|Rotation Mode", Replicated)
	FGameplayTag DesiredRotationMode;

public:
	/**
	 * Return current DesiredRotationMode
	 */
	const FGameplayTag& GetDesiredRotationMode() const { return DesiredRotationMode; }

	/**
	 * Set current DesiredRotationMode
	 */
	UFUNCTION(BlueprintCallable, Category = "State|Rotation Mode")
	void SetDesiredRotationMode(FGameplayTag NewDesiredRotationMode);

private:
	/**
	 * Set current DesiredRotationMode (SERVER)
	 */
	UFUNCTION(Server, Reliable)
	void Server_SetDesiredRotationMode(FGameplayTag NewDesiredRotationMode);
	void Server_SetDesiredRotationMode_Implementation(FGameplayTag NewDesiredRotationMode);

#pragma endregion


	////////////////////////////////////////////////
	// Rotation Mode
#pragma region Rotation Mode
protected:
	//
	// Current rotation mode of Character
	// (Velocity Direction, View Direction, Aiming ...)
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Rotation Mode", Transient)
	FGameplayTag RotationMode;

public:
	/**
	 * Get current RotationMode
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State|Rotation Mode")
	const FGameplayTag& GetRotationMode() const { return RotationMode; }

protected:
	/**
	 * Set current RotationMode
	 */
	void SetRotationMode(FGameplayTag NewRotationMode);

#pragma endregion


	////////////////////////////////////////////////
	// Desired Stance
#pragma region Desired Stance
protected:
	//
	// Current stance state for which a transition is desired
	// (Standing, Crouching ...)
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs|Stance", Replicated)
	FGameplayTag DesiredStance;

public:
	/**
	 * Get current DesiredStance
	 */
	const FGameplayTag& GetDesiredStance() const { return DesiredStance; }

	/**
	 * Set current DesiredStance
	 */
	UFUNCTION(BlueprintCallable, Category = "State|Stance")
	void SetDesiredStance(FGameplayTag NewDesiredStance);

private:
	/**
	 * Set current DesiredStance (SERVER)
	 */
	UFUNCTION(Server, Reliable)
	void Server_SetDesiredStance(FGameplayTag NewDesiredStance);
	void Server_SetDesiredStance_Implementation(FGameplayTag NewDesiredStance);

#pragma endregion


	////////////////////////////////////////////////
	// Stance
#pragma region Stance
protected:
	//
	// Current Stance state of the Character
	// (Standing, Crouching ...)
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Stance", Transient)
	FGameplayTag Stance;

public:
	virtual bool CanCrouchInCurrentState() const override;

	/**
	 * Get current Stance
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State|Stance")
	const FGameplayTag& GetStance() const { return Stance; }

	/**
	 * Set current Stance
	 */
	void SetStance(FGameplayTag NewStance);

#pragma endregion


	////////////////////////////////////////////////
	// Desired Gait
#pragma region Desired Gait
protected:
	//
	// Current gait state in which the transition is desired
	// (Walk, Run, Sprint ...)
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configs|Gait", Replicated)
	FGameplayTag DesiredGait;

public:
	/**
	 * Get current DesiredGait
	 */
	FGameplayTag GetDesiredGait() const { return DesiredGait; }

	/**
	 * Set current DesiredGait
	 */
	UFUNCTION(BlueprintCallable, Category = "State|Gait")
	void SetDesiredGait(FGameplayTag NewDesiredGait);

private:
	/**
	 * Set current DesiredGait (SERVER)
	 */
	UFUNCTION(Server, Reliable)
	void Server_SetDesiredGait(FGameplayTag NewDesiredGait);
	void Server_SetDesiredGait_Implementation(FGameplayTag NewDesiredGait);

#pragma endregion


	////////////////////////////////////////////////
	// Gait
#pragma region Gait
protected:
	//
	// Maximum Gait state of Character
	// (Walk, Run, Sprint ...)
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Gait", Transient)
	FGameplayTag Gait;

	//
	// Max movement speed speed in the current Gait state of Character
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Gait", Transient)
	float MaxSpeed;

	//
	// Braking deceleration in the current Gait state of Character
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Gait", Transient)
	float BrakingDeceleration;

	//
	// Rotational interpolation speed in the current Gait state of Character
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Gait", Transient)
	float RotationInterpSpeed;

	//
	// Whether GaitConfigs should be updated
	//
	UPROPERTY(BlueprintReadWrite, Transient, VisibleAnywhere, Category = "State|Gait")
	bool bShouldUpdateGaitConfigs{ true };

public:
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float InBrakingDeceleration) override;

	virtual float GetMaxSpeed() const override { return MaxSpeed; }
	virtual float GetMaxBrakingDeceleration() const override { return BrakingDeceleration; }

	/**
	 * Get current Gait
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State|Gait")
	FGameplayTag GetGait() const { return Gait; }

protected:
	/**
	 * Set current Gait
	 */
	void SetGait(FGameplayTag NewGait);

	/**
	 * Update GaitConfigs corresponding to the current Gait
	 */
	void RefreshGaitConfigs();
	void RefreshGaitConfigs(const FCharacterGaitConfigs& InGaitConfigs);

private:
	/**
	 * Get the rotation speed of a MovementBase.
	 */
	bool TryGetMovementBaseRotationSpeed(const FBasedMovementInfo& BasedMovement, FRotator& RotationSpeed);

#pragma endregion


	//////////////////////////////////////////
	// Locomotion Action
#pragma region Locomotion Action
protected:
	//
	// Current action of Character
	// (Mantle, Dodge ...)
	// 
	// Tips:
	//	Basically, it is set via Notify in the AnimMontage that is played.
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Locomotion Action", Transient)
	FGameplayTag LocomotionAction;

public:
	/**
	 * Get current LocomotionAction
	 */
	const FGameplayTag& GetLocomotionAction() const { return LocomotionAction; }

	/**
	 * Set current LocomotionAction
	 */
	void SetLocomotionAction(const FGameplayTag& NewLocomotionAction);

#pragma endregion


	//////////////////////////////////////////
	// Input
#pragma region Input
protected:
	//
	// Replicated input direction
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Input", Transient, Replicated)
	FVector_NetQuantizeNormal InputDirection;

public:
	/**
	 * Get current InputDirection
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State|Input")
	const FVector& GetInputDirection() const { return InputDirection; }

protected:
	virtual FVector ConsumeInputVector() override;

	/**
	 * Set current InputDirection
	 */
	void SetInputDirection(FVector NewInputDirection);

	/**
	 * Update the current InputDirection
	 */
	void UpdateInput(float DeltaTime);

#pragma endregion


	////////////////////////////////////////////////
	// View
#pragma region View
protected:
	//
	// Raw viewpoint rotation information replicated in the network.
	// 
	// Tips:
	//	Basically, use FViewState::Rotation, which takes advantage of network smoothing.
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|View", Transient, ReplicatedUsing = "OnReplicated_ReplicatedViewRotation")
	FRotator ReplicatedViewRotation;

	//
	// Current view state of Character
	// (回転, 速度 ...)
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|View", Transient)
	FViewState ViewState;

public:
	/**
	 * Get current ViewState
	 */
	const FViewState& GetViewState() const { return ViewState; }

protected:
	/**
	 * Update the current ViewState
	 */
	void UpdateView(float DeltaTime);

private:
	/**
	 * Modify the current ViewState.Rotation based on network smoothing
	 */
	void CorrectViewNetworkSmoothing(const FRotator& NewViewRotation);

	/**
	 * Update current ViewState based on network smoothing
	 */
	void UpdateViewNetworkSmoothing(float DeltaTime);

	/**
	 * Set current ReplicatedViewRotation
	 */
	void SetReplicatedViewRotation(const FRotator& NewViewRotation);

	/**
	 * Notify that ReplicatedViewRotation has been replicated.
	 */
	UFUNCTION()
	void OnReplicated_ReplicatedViewRotation();

	/**
	 * Set current ReplicatedViewRotation (SERVER)
	 */
	UFUNCTION(Server, Unreliable)
	void Server_SetReplicatedViewRotation(const FRotator& NewViewRotation);
	void Server_SetReplicatedViewRotation_Implementation(const FRotator& NewViewRotation);

#pragma endregion


	////////////////////////////////////////////////
	// Locomotion State
#pragma region Locomotion State
protected:
	//
	// Current locomotion state of character
	// (Input, Move speed, Acceleration ...)
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FLocomotionState LocomotionState;

	//
	// Direction angle of replicated travel speed
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Replicated, Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float DesiredVelocityYawAngle;

	//
	// The previous update of ControlRotation
	// 
	// Tips:
	//	Only Locally Controlled Character is updated.
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FRotator PreviousControlRotation;

	//
	// Correction of the position at the time of penetration
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FVector PendingPenetrationAdjustment;

	//
	// Correction of the velocity at the time of penetration
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FVector PrePenetrationAdjustmentVelocity;

	//
	// Whether the speed before position correction is valid or not at the time of penetration
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	bool bPrePenetrationAdjustmentVelocityValid;

public:
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;
	virtual void ComputeFloorDist(
		const FVector& CapsuleLocation,
		float LineDistance,
		float SweepDistance,
		FFindFloorResult& OutFloorResult,
		float SweepRadius,
		const FHitResult* DownwardSweepResult) const override;

	/**
	 * Get current LocomotionState
	 */
	const FLocomotionState& GetLocomotionState() const { return LocomotionState; }

protected:
	virtual bool CanAttemptJump() const override;

	virtual void ControlledCharacterMove(const FVector& InputVector, float DeltaTime) override;
	virtual void PhysWalking(float DeltaTime, int32 Iterations) override;
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	virtual void PerformMovement(float DeltaTime) override;
	virtual void SmoothClientPosition(float DeltaTime) override;
	virtual void MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAcceleration) override;

	bool TryConsumePrePenetrationAdjustmentVelocity(FVector& OutVelocity);

	/**
	 * Moving updates performed before other updates
	 */
	virtual void UpdateLocomotionEarly();

	/**
	 * Moving updates performed between other updates
	 */
	virtual void UpdateLocomotion(float DeltaTime);

	/**
	 * Moving updates performed after other updates
	 */
	virtual void UpdateLocomotionLate(float DeltaTime);

	/**
	 * Update movement and rotation
	 */
	virtual void UpdateLocomotionLocationAndRotation();

private:
	void SavePenetrationAdjustment(const FHitResult& Hit);
	void ApplyPendingPenetrationAdjustment();

	/**
	 * Set current DesiredVelocityYawAngle
	 */
	void SetDesiredVelocityYawAngle(float NewDesiredVelocityYawAngle);

#pragma endregion


	//////////////////////////////////////////
	// Rotation
#pragma region Rotation
public:
	/**
	 * Performs character rotation when on the ground.
	 */
	void UpdateOnGroundRotation(float DeltaTime);

	/**
	 * Performs Character rotation when in the air.
	 */
	void UpdateInAirRotation(float DeltaTime);

	/**
	 * Performs Character rotation when in the water.
	 */
	void UpdateInWaterRotation(float DeltaTime);

protected:
	/**
	 * Calculate interpolation speed of rotation
	 */
	float CalculateRotationInterpolationSpeed() const;

	/**
	 * Update rotation
	 */
	void UpdateRotation(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed);

	/**
	 * Smoothly update rotation
	 */
	void UpdateRotationExtraSmooth(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed, float TargetYawAngleRotationSpeed);

	/**
	 * Instantaneous update of rotation
	 */
	void UpdateRotationInstant(float TargetYawAngle, ETeleportType Teleport = ETeleportType::None);

	/**
	 * Update the angle of the view using move-rotate
	 */
	void UpdateTargetYawAngleUsingLocomotionRotation();

	/**
	 * Update the angle of view
	 */
	void UpdateTargetYawAngle(float TargetYawAngle);

	/**
	 * Update the relative angle of view
	 */
	void UpdateViewRelativeTargetYawAngle();

private:
	/**
	 * Apply the rotation speed of the view
	 */
	void ApplyRotationYawSpeed(float DeltaTime);

#pragma endregion


	//////////////////////////////////////////
	// Utilities
#pragma region Utilities
public:
	template<typename T = ALocomotionCharacter>
	T* GetCharacter() const
	{
		return Cast<T>(CharacterOwner);
	}

	template<typename T = ALocomotionCharacter>
	T* GetCharacterChecked() const
	{
		return CastChecked<T>(CharacterOwner);
	}

	UFUNCTION(BlueprintPure, Category = "Character")
	static ULocomotionComponent* FindLocomotionComponent(const ACharacter* Character);
	
#pragma endregion

};

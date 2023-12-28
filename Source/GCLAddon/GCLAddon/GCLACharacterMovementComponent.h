// Copyright (C) 2023 owoDra

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"

#include "Movement/State/ViewState.h"
#include "Movement/State/LocomotionState.h"
#include "Movement/State/MovementBaseState.h"
#include "Movement/CharacterMovementConfigs.h"

#include "GameplayTagContainer.h"

#include "GCLACharacterMovementComponent.generated.h"

class UCharacterMovementData;
struct FBasedMovementInfo;


/**
 * FGCLACharacterNetworkMoveData
 */
class FGCLACharacterNetworkMoveData : public FCharacterNetworkMoveData
{
public:
	FGCLACharacterNetworkMoveData();

private:
	using Super = FCharacterNetworkMoveData;

public:
	FGameplayTag RotationMode;
	FGameplayTag Stance;
	FGameplayTag Gait;

public:
	virtual void ClientFillNetworkMoveData(const FSavedMove_Character& Move, ENetworkMoveType MoveType) override;
	virtual bool Serialize(UCharacterMovementComponent& Movement, FArchive& Archive, UPackageMap* Map, ENetworkMoveType MoveType) override;

};


/**
 * FGCLACharacterNetworkMoveDataContainer
 */
class FGCLACharacterNetworkMoveDataContainer : public FCharacterNetworkMoveDataContainer
{
public:
	FGCLACharacterNetworkMoveDataContainer();

public:
	FGCLACharacterNetworkMoveData MoveData[3];
};


/**
 * FGCLASavedMove
 */
class FGCLASavedMove : public FSavedMove_Character
{
public:
	FGCLASavedMove();

private:
	using Super = FSavedMove_Character;

public:
	FGameplayTag RotationMode;
	FGameplayTag Stance;
	FGameplayTag Gait;

public:
	virtual void Clear() override;

	virtual void SetMoveFor(ACharacter* Character, float NewDeltaTime, const FVector& NewAcceleration,
		FNetworkPredictionData_Client_Character& PredictionData) override;

	virtual bool CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* Character, float MaxDelta) const override;

	virtual void CombineWith(const FSavedMove_Character* PreviousMove, ACharacter* Character,
		APlayerController* Player, const FVector& PreviousStartLocation) override;

	virtual void PrepMoveFor(ACharacter* Character) override;
};


/**
 * FGCLANetworkPredictionData
 */
class FGCLANetworkPredictionData : public FNetworkPredictionData_Client_Character
{
private:
	using Super = FNetworkPredictionData_Client_Character;

public:
	explicit FGCLANetworkPredictionData(const UGCLACharacterMovementComponent& Movement);

	virtual FSavedMovePtr AllocateNewMove() override;
};


/**
 * Components that extend the processing of character movement relationships
 */
UCLASS()
class GCLADDON_API UGCLACharacterMovementComponent 
	: public UCharacterMovementComponent
	, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	friend FGCLASavedMove;
	
public:
	UGCLACharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	//
	// Function name used to add this component
	// 
	static const FName NAME_ActorFeatureName;


	////////////////////////////////////////////////
	// Initialization
protected:
	FGCLACharacterNetworkMoveDataContainer MoveDataContainer;

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


	////////////////////////////////////////////////
	// Movement Data
protected:
	//
	// Current movement data
	//
	UPROPERTY(EditAnywhere, ReplicatedUsing = "OnRep_MovementData")
	TObjectPtr<const UCharacterMovementData> MovementData;

protected:
	UFUNCTION()
	virtual void OnRep_MovementData();

	/**
	 * Apply the current movement data
	 */
	virtual void ApplyMovementData();

public:
	/**
	 * Set the current movement data
	 */
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
	void SetMovementData(const UCharacterMovementData* NewMovementData);


	////////////////////////////////////////////////
	// Movement Base
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


	//////////////////////////////////////////
	// Locomotion Mode
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


	////////////////////////////////////////////////
	// Desired Rotation Mode
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


	////////////////////////////////////////////////
	// Rotation Mode
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


	////////////////////////////////////////////////
	// Desired Stance
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


	////////////////////////////////////////////////
	// Stance
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


	////////////////////////////////////////////////
	// Desired Gait
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


	////////////////////////////////////////////////
	// Gait
protected:
	//
	// Maximum Gait state of Character
	// (Walk, Run, Sprint ...)
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Gait", Transient)
	FGameplayTag Gait;

	//
	// Rotational interpolation speed in the current Gait state of Character
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State|Gait", Transient)
	float RotationInterpSpeed{ 0.0 };

	//
	// Whether GaitConfigs should be updated
	//
	UPROPERTY(BlueprintReadWrite, Transient, VisibleAnywhere, Category = "State|Gait")
	bool bShouldUpdateGaitConfigs{ true };

public:
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;

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
	void RefreshGaitConfigs(const FCharacterGaitConfigs& GaitConfigs);

private:
	/**
	 * Get the rotation speed of a MovementBase.
	 */
	bool TryGetMovementBaseRotationSpeed(const FBasedMovementInfo& BasedMovement, FRotator& RotationSpeed);


	//////////////////////////////////////////
	// Locomotion Action
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


	//////////////////////////////////////////
	// Input
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


	////////////////////////////////////////////////
	// View
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


	////////////////////////////////////////////////
	// Locomotion State
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


	//////////////////////////////////////////
	// Rotation
public:
	/**
	 * Performs character rotation when on the ground.
	 */
	void UpdateGroundedRotation(float DeltaTime);

	/**
	 * Performs Character rotation when in the air.
	 */
	void UpdateInAirRotation(float DeltaTime);

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


	//////////////////////////////////////////
	// Utilities
public:
	template<typename T = AGCLACharacter>
	T* GetCharacter() const
	{
		return Cast<T>(CharacterOwner);
	}

	template<typename T = AGCLACharacter>
	T* GetCharacterChecked() const
	{
		return CastChecked<T>(CharacterOwner);
	}

	/**
	 *	Character からこのコンポーネントを取得する
	 */
	UFUNCTION(BlueprintPure, Category = "Character")
	static UGCLACharacterMovementComponent* FindGCLACharacterMovementComponent(const ACharacter* Character);

};

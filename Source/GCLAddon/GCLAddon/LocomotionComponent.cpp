// Copyright (C) 2023 owoDra

#include "LocomotionComponent.h"

#include "CharacterAnimInstance.h"
#include "CustomMovement/CustomMovementProcess.h"
#include "GameplayTag/GCLATags_Status.h"
#include "LocomotionGeneralNameStatics.h"
#include "LocomotionFunctionLibrary.h"
#include "LocomotionCharacter.h"
#include "LocomotionData.h"
#include "GCLAddonLogs.h"
#include "GCLAddonStatGroup.h"

#include "CharacterMeshAccessorInterface.h"

#include "InitState/InitStateComponent.h"
#include "InitState/InitStateTags.h"

#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "GameFramework/GameNetworkManager.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LocomotionComponent)


const FName ULocomotionComponent::NAME_ActorFeatureName("Locomotion");

ULocomotionComponent::ULocomotionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetNetworkMoveDataContainer(MoveDataContainer);

	// Setup general Settings

	GravityScale = 1.0f;
	MaxAcceleration = 2400.0f;
	BrakingFrictionFactor = 1.0f;
	BrakingFriction = 6.0f;
	GroundFriction = 8.0f;
	RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	bUseControllerDesiredRotation = false;
	bOrientRotationToMovement = false;
	bNetworkAlwaysReplicateTransformUpdateTimestamp = true;
	bAllowPhysicsRotationDuringAnimRootMotion = false;
	bCanWalkOffLedgesWhenCrouching = true;
	NavAgentProps.bCanCrouch = true;
	NavAgentProps.bCanFly = true;
	bUseAccelerationForPaths = true;
	SetCrouchedHalfHeight(65.0f);

	// Setup states

	LocomotionMode			= TAG_Status_LocomotionMode_OnGround;
	DesiredRotationMode		= TAG_Status_RotationMode_ViewDirection;
	RotationMode			= TAG_Status_RotationMode_ViewDirection;
	DesiredStance			= TAG_Status_Stance_Standing;
	Stance					= TAG_Status_Stance_Standing;
	DesiredGait				= TAG_Status_Gait_Walking;
	Gait					= TAG_Status_Gait_Walking;
}

#if WITH_EDITOR
bool ULocomotionComponent::CanEditChange(const FProperty* Property) const
{
	return Super::CanEditChange(Property) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, RotationRate) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, bUseControllerDesiredRotation) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, bOrientRotationToMovement);
}
#endif

void ULocomotionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;

	Parameters.Condition = COND_SkipOwner;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DesiredStance, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DesiredGait, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DesiredRotationMode, Parameters);

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedViewRotation, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InputDirection, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DesiredVelocityYawAngle, Parameters);

	DOREPLIFETIME(ThisClass, LocomotionData);
}

FNetworkPredictionData_Client* ULocomotionComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		auto* MutableThis{ const_cast<ULocomotionComponent*>(this) };

		MutableThis->ClientPredictionData = new FLocomotionNetworkPredictionData(*this);
	}

	return ClientPredictionData;
}


#pragma region Initialize and Deinitialize

void ULocomotionComponent::OnRegister()
{
	Super::OnRegister();

	// This component can only be added to classes derived from ALocomotionCharacter

	const auto* Character{ GetOwner<ALocomotionCharacter>() };
	ensureAlwaysMsgf((Character != nullptr), TEXT("[%s] on [%s] can only be added to LocomotionCharacter actors."), *GetNameSafe(StaticClass()), *GetNameSafe(GetOwner()));


	// No more than two of these components should be added to a Pawn.

	TArray<UActorComponent*> Components;
	GetOwner()->GetComponents(StaticClass(), Components);
	ensureAlwaysMsgf((Components.Num() == 1), TEXT("Only one [%s] should exist on [%s]."), *GetNameSafe(StaticClass()), *GetNameSafe(GetOwner()));

	// Register this component in the GameFrameworkComponentManager.

	RegisterInitStateFeature();
}

void ULocomotionComponent::BeginPlay()
{
	ensureMsgf(!bUseControllerDesiredRotation && !bOrientRotationToMovement, TEXT("bUseControllerDesiredRotation and bOrientRotationToMovement must be turned false!"));

	Super::BeginPlay();

	// Start listening for changes in the initialization state of all features 
	// related to the Pawn that owns this component.

	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// Change the initialization state of this component to [Spawned]

	ensure(TryToChangeInitState(TAG_InitState_Spawned));

	// Check if initialization process can continue

	CheckDefaultInitialization();
}

void ULocomotionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}


bool ULocomotionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	/**
	 * [None] -> [Spawned]
	 */
	if (!CurrentState.IsValid() && DesiredState == TAG_InitState_Spawned)
	{
		if (GetOwner() == nullptr)
		{
			return false;
		}

		return CanChangeInitStateToSpawned(Manager);
	}

	/**
	 * [Spawned] -> [DataAvailable]
	 */
	else if (CurrentState == TAG_InitState_Spawned && DesiredState == TAG_InitState_DataAvailable)
	{
		if (!Manager->HasFeatureReachedInitState(GetOwner(), UInitStateComponent::NAME_ActorFeatureName, TAG_InitState_DataAvailable))
		{
			return false;
		}

		return CanChangeInitStateToDataAvailable(Manager);
	}

	/**
	 * [DataAvailable] -> [DataInitialized]
	 */
	else if (CurrentState == TAG_InitState_DataAvailable && DesiredState == TAG_InitState_DataInitialized)
	{
		if (LocomotionData == nullptr)
		{
			return false;
		}

		return CanChangeInitStateToDataInitialized(Manager);
	}

	/**
	 * [DataInitialized] -> [GameplayReady]
	 */
	else if (CurrentState == TAG_InitState_DataInitialized && DesiredState == TAG_InitState_GameplayReady)
	{
		return CanChangeInitStateToGameplayReady(Manager);
	}

	return false;
}

void ULocomotionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	UE_LOG(LogGCLA, Log, TEXT("[%s] Character Movement Component InitState Reached: %s"),
		GetOwner()->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"), *DesiredState.GetTagName().ToString());

	/**
	 * [InitState None] -> [InitState Spawned]
	 */
	if (!CurrentState.IsValid() && DesiredState == TAG_InitState_Spawned)
	{
		HandleChangeInitStateToSpawned(Manager);
	}

	/**
	 * [InitState Spawned] -> [InitState DataAvailable]
	 */
	else if (CurrentState == TAG_InitState_Spawned && DesiredState == TAG_InitState_DataAvailable)
	{
		HandleChangeInitStateToDataAvailable(Manager);
	}

	/**
	 * [InitState DataAvailable] -> [InitState DataInitialized]
	 */
	else if (CurrentState == TAG_InitState_DataAvailable && DesiredState == TAG_InitState_DataInitialized)
	{
		ApplyLocomotionData();

		HandleChangeInitStateToDataInitialized(Manager);
	}

	/**
	 * [InitState DataInitialized] -> [InitState GameplayReady]
	 */
	else if (CurrentState == TAG_InitState_DataInitialized && DesiredState == TAG_InitState_GameplayReady)
	{
		HandleChangeInitStateToGameplayReady(Manager);
	}
}

void ULocomotionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UInitStateComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == TAG_InitState_DataAvailable)
		{
			CheckDefaultInitialization();
		}
	}
}

void ULocomotionComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain
	{
		TAG_InitState_Spawned,
		TAG_InitState_DataAvailable,
		TAG_InitState_DataInitialized,
		TAG_InitState_GameplayReady
	};

	ContinueInitStateChain(StateChain);
}

#pragma endregion


#pragma region Movement Data

void ULocomotionComponent::OnRep_LocomotionData()
{
	CheckDefaultInitialization();
}

void ULocomotionComponent::ApplyLocomotionData()
{
	check(LocomotionData);

	auto* LocomotionCharacter{ GetCharacterChecked<ALocomotionCharacter>() };

	CreateCustomMovementProcesses();

	UpdateLocomotionConfigs();

	SetReplicatedViewRotation(LocomotionCharacter->GetViewRotationSuperClass());

	ViewState.NetworkSmoothing.InitialRotation = ReplicatedViewRotation;
	ViewState.NetworkSmoothing.Rotation = ReplicatedViewRotation;
	ViewState.Rotation = ReplicatedViewRotation;
	ViewState.PreviousYawAngle = UE_REAL_TO_FLOAT(ReplicatedViewRotation.Yaw);

	const auto& ActorTransform{ CharacterOwner->GetActorTransform() };

	LocomotionState.Location = ActorTransform.GetLocation();
	LocomotionState.RotationQuaternion = ActorTransform.GetRotation();
	LocomotionState.Rotation = ActorTransform.GetRotation().Rotator();
	LocomotionState.PreviousYawAngle = UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw);

	UpdateTargetYawAngleUsingLocomotionRotation();

	LocomotionState.InputYawAngle = UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw);
	LocomotionState.VelocityYawAngle = UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw);
}

void ULocomotionComponent::CreateCustomMovementProcesses()
{
	CustomMovementProcesses.Empty();

	auto SoftClassesMap{ LocomotionData->CustomMovementProcesses };
	for (const auto& KVP : SoftClassesMap)
	{
		const auto& CustomMoveIdx{ KVP.Key };
		const auto& SoftClass{ KVP.Value };

		if (SoftClass.IsNull())
		{
			continue;
		}

		const auto* ProcessClass{ SoftClass.LoadSynchronous() };

		if (!ProcessClass)
		{
			continue;
		}

		auto* NewProcess{ NewObject<UCustomMovementProcess>(this, ProcessClass) };

		if (!NewProcess)
		{
			continue;
		}

		CustomMovementProcesses.Add(CustomMoveIdx, NewProcess);
	}
}

void ULocomotionComponent::SetLocomotionData(const ULocomotionData* NewLocomotionData)
{
	if (GetOwner()->HasAuthority())
	{
		if (NewLocomotionData != LocomotionData)
		{
			LocomotionData = NewLocomotionData;

			CheckDefaultInitialization();
		}
	}
}

#pragma endregion


#pragma region Movement Base

void ULocomotionComponent::UpdateBasedRotation(FRotator& FinalRotation, const FRotator& ReducedRotation)
{
	const auto& BasedMovement{ CharacterOwner->GetBasedMovement() };

	FVector MovementBaseLocation;
	FQuat MovementBaseRotation;

	MovementBaseUtility::GetMovementBaseTransform(BasedMovement.MovementBase, BasedMovement.BoneName, MovementBaseLocation, MovementBaseRotation);

	if (!OldBaseQuat.Equals(MovementBaseRotation, UE_SMALL_NUMBER))
	{
		const auto DeltaRotation{ (MovementBaseRotation * OldBaseQuat.Inverse()).Rotator() };
		auto NewControlRotation{ CharacterOwner->Controller->GetControlRotation() };

		NewControlRotation.Pitch += DeltaRotation.Pitch;
		NewControlRotation.Yaw += DeltaRotation.Yaw;
		NewControlRotation.Normalize();

		CharacterOwner->Controller->SetControlRotation(NewControlRotation);
	}
}

void ULocomotionComponent::UpdateUsingAbsoluteRotation() const
{
	const auto bNotDedicatedServer{ !IsNetMode(NM_DedicatedServer) };

	const auto bAutonomousProxyOnListenServer{ IsNetMode(NM_ListenServer) && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy };

	const auto bNonLocallyControllerCharacterWithURO{ CharacterOwner->GetMesh()->ShouldUseUpdateRateOptimizations() && !IsValid(CharacterOwner->GetInstigatorController<APlayerController>()) };

	CharacterOwner->GetMesh()->SetUsingAbsoluteRotation(bNotDedicatedServer && (bAutonomousProxyOnListenServer || bNonLocallyControllerCharacterWithURO));
}

void ULocomotionComponent::UpdateVisibilityBasedAnimTickOption() const
{
	const auto DefaultTickOption{ CharacterOwner->GetClass()->GetDefaultObject<ACharacter>()->GetMesh()->VisibilityBasedAnimTickOption };

	const auto TargetTickOption
	{ 
		(IsNetMode(NM_Standalone) || CharacterOwner->GetLocalRole() <= ROLE_AutonomousProxy || CharacterOwner->GetRemoteRole() != ROLE_AutonomousProxy)
		? EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered : EVisibilityBasedAnimTickOption::AlwaysTickPose
	};

	CharacterOwner->GetMesh()->VisibilityBasedAnimTickOption = (TargetTickOption <= DefaultTickOption) ? TargetTickOption : DefaultTickOption;
}

void ULocomotionComponent::UpdateMovementBase()
{
	const auto& BasedMovement{ CharacterOwner->GetBasedMovement() };

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

	MovementBase.DeltaRotation = (MovementBase.bHasRelativeLocation && !MovementBase.bBaseChanged) ? (MovementBase.Rotation * PreviousRotation.Inverse()).Rotator() : FRotator::ZeroRotator;
}

void ULocomotionComponent::UpdateAnimInstanceMovement()
{
	if (!CharacterOwner->GetMesh()->bRecentlyRendered &&
		(CharacterOwner->GetMesh()->VisibilityBasedAnimTickOption > EVisibilityBasedAnimTickOption::AlwaysTickPose))
	{
		if (auto* MainMesh{ ICharacterMeshAccessorInterface::Execute_GetMainMesh(CharacterOwner) })
		{
			if (auto* AnimIns{ Cast<UCharacterAnimInstance>(MainMesh->GetAnimInstance()) })
			{
				AnimIns->MarkPendingUpdate();
			}
		}
	}
}

#pragma endregion


#pragma region Locomotion Mode

void ULocomotionComponent::SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode)
{
	if (!bMovementModeLocked)
	{
		// Execute the processing of the parent function while LocomotionData is not assigned.

		if (!LocomotionData)
		{
			Super::SetMovementMode(NewMovementMode, NewCustomMode);
		}

		// Execute parent function if transition is possible based on LocomotionData data.

		else if (LocomotionData->CanChangeMovementModeTo(this, NewMovementMode, NewCustomMode))
		{
			Super::SetMovementMode(NewMovementMode, NewCustomMode);
		}
	}
}

void ULocomotionComponent::SetLocomotionMode(const FGameplayTag& NewLocomotionMode)
{
	if (LocomotionMode != NewLocomotionMode)
	{
		LocomotionMode = NewLocomotionMode;

		bShouldUpdateGaitConfigs = true;
	}
}

void ULocomotionComponent::UpdateLocomotionConfigs()
{
	// Get Configs for current LocomotionMode

	const auto& LocomotionModeConfigs{ LocomotionData->GetLocomotionModeConfig(LocomotionMode) };

	// Get Tag and Configs from LocomotionModeConfigs based on current DesiredRotationMode and update RotationMode

	auto AllowedRotationMode{ DesiredRotationMode };
	const auto& RotationModeConfigs{ LocomotionModeConfigs.GetAllowedRotationMode(this, DesiredRotationMode, AllowedRotationMode) };
	SetRotationMode(AllowedRotationMode);
	
	// Get current Stance Configs from RotationModeConfigs

	auto AllowedStance{ DesiredStance };
	const auto& StanceConfigs{ RotationModeConfigs.GetAllowedStance(this, DesiredStance, AllowedStance) };
	SetStance(AllowedStance);

	// Get Tag and Configs from StanceConfigs based on current DesiredGait Update Gait

	auto AllowedGait{ DesiredGait };
	const auto& GaitConfigs{ StanceConfigs.GetAllowedGait(this, DesiredGait, AllowedGait) };
	SetGait(AllowedGait);

	if (bShouldUpdateGaitConfigs)
	{
		RefreshGaitConfigs(GaitConfigs);

		bShouldUpdateGaitConfigs = false;
	}
}

void ULocomotionComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	bCrouchMaintainsBaseLocation = true;

	if (LocomotionData)
	{
		// Set new locomotion mode

		const auto LocomotionModeTag{ LocomotionData->Convert_MovementModeToLocomotionMode(MovementMode, CustomMovementMode) };

		if (LocomotionModeTag.IsValid())
		{
			SetLocomotionMode(LocomotionModeTag);
		}
		else
		{
			UE_LOG(LogGCLA, Error, TEXT("No LocomotionMode matching the current MovementMode(%d, %u) was found in LocomotionData(%s)"),
				static_cast<int>(MovementMode), CustomMovementMode, *GetNameSafe(LocomotionData));
		}

		// If the MovementMode is Custom

		if (MovementMode == MOVE_Custom)
		{
			if (auto Process{ CustomMovementProcesses.FindRef(CustomMovementMode) })
			{
				Process->OnMovementStart(this);
			}
		}

		// If the previous MovementMode was Custom

		if (PreviousMovementMode == MOVE_Custom)
		{
			if (auto Process{ CustomMovementProcesses.FindRef(PreviousCustomMode) })
			{
				Process->OnMovementEnd(this);
			}
		}
	}
}

#pragma endregion


#pragma region Desired Rotation Mode

void ULocomotionComponent::SetDesiredRotationMode(FGameplayTag NewDesiredRotationMode)
{
	if (DesiredRotationMode != NewDesiredRotationMode)
	{
		DesiredRotationMode = NewDesiredRotationMode;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DesiredRotationMode, this);

		if (CharacterOwner->GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetDesiredRotationMode(DesiredRotationMode);
		}
	}
}

void ULocomotionComponent::Server_SetDesiredRotationMode_Implementation(FGameplayTag NewDesiredRotationMode)
{
	SetDesiredRotationMode(NewDesiredRotationMode);
}

#pragma endregion


#pragma region Rotation Mode

void ULocomotionComponent::SetRotationMode(FGameplayTag NewRotationMode)
{
	if (RotationMode != NewRotationMode)
	{
		RotationMode = NewRotationMode;

		bShouldUpdateGaitConfigs = true;
	}
}

#pragma endregion


#pragma region Desired Stance

void ULocomotionComponent::SetDesiredStance(FGameplayTag NewDesiredStance)
{
	if (DesiredStance != NewDesiredStance)
	{
		DesiredStance = NewDesiredStance;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DesiredStance, this);

		if (CharacterOwner->GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetDesiredStance(DesiredStance);
		}
	}
}

void ULocomotionComponent::Server_SetDesiredStance_Implementation(FGameplayTag NewDesiredStance)
{
	SetDesiredStance(NewDesiredStance);
}

#pragma endregion 


#pragma region Stance

void ULocomotionComponent::SetStance(FGameplayTag NewStance)
{
	if (NewStance == TAG_Status_Stance_Standing)
	{
		CharacterOwner->UnCrouch();
	}
	else if (NewStance == TAG_Status_Stance_Crouching)
	{
		CharacterOwner->Crouch();
	}

	// Override DesiredStance accordingly if the character is not yet in crouching or standing

	NewStance = IsCrouching() ? TAG_Status_Stance_Crouching : TAG_Status_Stance_Standing;

	if (Stance != NewStance)
	{
		Stance = NewStance;

		bShouldUpdateGaitConfigs = true;
	}
}

bool ULocomotionComponent::CanCrouchInCurrentState() const
{
	if (!CanEverCrouch())
	{
		return false;
	}

	return UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

#pragma endregion


#pragma region Desired Gait

void ULocomotionComponent::SetDesiredGait(FGameplayTag NewDesiredGait)
{
	if (DesiredGait != NewDesiredGait)
	{
		DesiredGait = NewDesiredGait;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DesiredGait, this);

		if (CharacterOwner->GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetDesiredGait(DesiredGait);
		}
	}
}

void ULocomotionComponent::Server_SetDesiredGait_Implementation(FGameplayTag NewDesiredGait)
{
	SetDesiredGait(NewDesiredGait);
}

#pragma endregion


#pragma region Gait

void ULocomotionComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	FRotator BaseRotationSpeed;

	if (!bIgnoreBaseRotation && TryGetMovementBaseRotationSpeed(CharacterOwner->GetBasedMovement(), BaseRotationSpeed))
	{
		// Offset the velocity to maintain the velocity relative to the base of the movement

		Velocity = (BaseRotationSpeed * DeltaTime).RotateVector(Velocity);
	}

	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
}


void ULocomotionComponent::SetGait(FGameplayTag NewGait)
{
	if (Gait != NewGait)
	{
		Gait = NewGait;

		bShouldUpdateGaitConfigs = true;
	}
}

void ULocomotionComponent::RefreshGaitConfigs()
{
	auto AllowedRotationMode{ RotationMode };
	auto AllowedStance{ Stance };
	auto AllowedGait{ Gait };

	const auto& LocomotionModeConfigs{ LocomotionData->GetLocomotionModeConfig(LocomotionMode) };
	const auto& RotationModeConfigs{ LocomotionModeConfigs.GetAllowedRotationMode(this, RotationMode, AllowedRotationMode) };
	const auto& StanceConfigs{ RotationModeConfigs.GetAllowedStance(this, Stance, AllowedStance) };
	const auto& GaitConfigs{ StanceConfigs.GetAllowedGait(this, Gait, AllowedGait) };

	RefreshGaitConfigs(GaitConfigs);
}

void ULocomotionComponent::RefreshGaitConfigs(const FCharacterGaitConfigs& GaitConfigs)
{
	MaxWalkSpeed			= GaitConfigs.MaxSpeed;
	MaxWalkSpeedCrouched	= GaitConfigs.MaxSpeed;
	MaxFlySpeed				= GaitConfigs.MaxSpeed;
	MaxSwimSpeed			= GaitConfigs.MaxSpeed;
	MaxCustomMovementSpeed	= GaitConfigs.MaxSpeed;

	MaxAcceleration = GaitConfigs.MaxAcceleration;

	BrakingDecelerationWalking	= GaitConfigs.BrakingDeceleration;
	BrakingDecelerationSwimming = GaitConfigs.BrakingDeceleration;
	BrakingDecelerationFlying	= GaitConfigs.BrakingDeceleration;
	BrakingDecelerationFalling	= GaitConfigs.BrakingDeceleration;

	GroundFriction = GaitConfigs.GroundFriction;

	JumpZVelocity = GaitConfigs.JumpZPower;

	AirControl = GaitConfigs.AirControl;

	RotationInterpSpeed = GaitConfigs.RotationInterpSpeed;
}


bool ULocomotionComponent::TryGetMovementBaseRotationSpeed(const FBasedMovementInfo& BasedMovement, FRotator& RotationSpeed)
{
	if (!MovementBaseUtility::IsDynamicBase(BasedMovement.MovementBase))
	{
		RotationSpeed = FRotator::ZeroRotator;
		return false;
	}

	const auto* BodyInstance{ BasedMovement.MovementBase->GetBodyInstance(BasedMovement.BoneName) };
	if (BodyInstance == nullptr)
	{
		RotationSpeed = FRotator::ZeroRotator;
		return false;
	}

	const auto AngularVelocityVector{ BodyInstance->GetUnrealWorldAngularVelocityInRadians() };
	if (AngularVelocityVector.IsNearlyZero())
	{
		RotationSpeed = FRotator::ZeroRotator;
		return false;
	}

	RotationSpeed.Roll	= FMath::RadiansToDegrees(AngularVelocityVector.X);
	RotationSpeed.Pitch = FMath::RadiansToDegrees(AngularVelocityVector.Y);
	RotationSpeed.Yaw	= FMath::RadiansToDegrees(AngularVelocityVector.Z);

	return true;
}

#pragma endregion


#pragma region Locomotion Action

void ULocomotionComponent::SetLocomotionAction(const FGameplayTag& NewLocomotionAction)
{
	if (LocomotionAction != NewLocomotionAction)
	{
		LocomotionAction = NewLocomotionAction;
	}
}

#pragma endregion


#pragma region Input

FVector ULocomotionComponent::ConsumeInputVector()
{
	auto InputVector{ Super::ConsumeInputVector() };

	FRotator BaseRotationSpeed;

	if (!bIgnoreBaseRotation && TryGetMovementBaseRotationSpeed(CharacterOwner->GetBasedMovement(), BaseRotationSpeed))
	{
		// Offset the input vector to maintain its value relative to the moving base

		InputVector = (BaseRotationSpeed * GetWorld()->GetDeltaSeconds()).RotateVector(InputVector);
	}

	return InputVector;
}

void ULocomotionComponent::SetInputDirection(FVector NewInputDirection)
{
	NewInputDirection = NewInputDirection.GetSafeNormal();

	COMPARE_ASSIGN_AND_MARK_PROPERTY_DIRTY(ThisClass, InputDirection, NewInputDirection, this);
}

void ULocomotionComponent::UpdateInput(float DeltaTime)
{
	if (CharacterOwner->GetLocalRole() >= ROLE_AutonomousProxy)
	{
		SetInputDirection(GetCurrentAcceleration() / GetMaxAcceleration());
	}

	LocomotionState.bHasInput = InputDirection.SizeSquared() > UE_KINDA_SMALL_NUMBER;

	if (LocomotionState.bHasInput)
	{
		LocomotionState.InputYawAngle = UE_REAL_TO_FLOAT(ULocomotionFunctionLibrary::DirectionToAngleXY(InputDirection));
	}
}

#pragma endregion


#pragma region View

void ULocomotionComponent::UpdateView(float DeltaTime)
{
	if (MovementBase.bHasRelativeRotation)
	{
		// Offset the rotations to keep them relative to the movement base.

		ReplicatedViewRotation.Pitch += MovementBase.DeltaRotation.Pitch;
		ReplicatedViewRotation.Yaw += MovementBase.DeltaRotation.Yaw;
		ReplicatedViewRotation.Normalize();

		ViewState.Rotation.Pitch += MovementBase.DeltaRotation.Pitch;
		ViewState.Rotation.Yaw += MovementBase.DeltaRotation.Yaw;
		ViewState.Rotation.Normalize();
	}

	ViewState.PreviousYawAngle = UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw);

	if ((CharacterOwner->IsReplicatingMovement() && CharacterOwner->GetLocalRole() >= ROLE_AutonomousProxy) || CharacterOwner->IsLocallyControlled())
	{
		SetReplicatedViewRotation(GetCharacterChecked()->GetViewRotationSuperClass());
	}

	UpdateViewNetworkSmoothing(DeltaTime);

	ViewState.Rotation = ViewState.NetworkSmoothing.Rotation;
	ViewState.YawSpeed = FMath::Abs(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw - ViewState.PreviousYawAngle)) / DeltaTime;
}

void ULocomotionComponent::CorrectViewNetworkSmoothing(const FRotator& NewViewRotation)
{
	ReplicatedViewRotation = NewViewRotation;
	ReplicatedViewRotation.Normalize();

	auto& NetworkSmoothing{ ViewState.NetworkSmoothing };

	if (!NetworkSmoothing.bEnabled)
	{
		NetworkSmoothing.InitialRotation = ReplicatedViewRotation;
		NetworkSmoothing.Rotation = ReplicatedViewRotation;
		return;
	}

	const auto bListenServer{ IsNetMode(NM_ListenServer) };
	const auto NewNetworkSmoothingServerTime{ bListenServer ? GetServerLastTransformUpdateTimeStamp() : CharacterOwner->GetReplicatedServerLastTransformUpdateTimeStamp() };

	if (NewNetworkSmoothingServerTime <= 0.0f)
	{
		return;
	}

	NetworkSmoothing.InitialRotation = NetworkSmoothing.Rotation;

	// Using server time, elapsed time can be known regardless of packet lag differences

	const auto ServerDeltaTime{ NewNetworkSmoothingServerTime - NetworkSmoothing.ServerTime };

	NetworkSmoothing.ServerTime = NewNetworkSmoothingServerTime;

	// Ensure that clients are not running significantly behind or ahead of the new server time

	const auto MaxServerDeltaTime{ GetDefault<AGameNetworkManager>()->MaxClientSmoothingDeltaTime };
	const auto MinServerDeltaTime{ FMath::Min(MaxServerDeltaTime, bListenServer ? ListenServerNetworkSimulatedSmoothLocationTime : NetworkSimulatedSmoothLocationTime) };

	// Calculate how much delay is possible after receiving the new server time

	const auto MinClientDeltaTime{ FMath::Clamp(ServerDeltaTime * 1.25f, MinServerDeltaTime, MaxServerDeltaTime) };

	NetworkSmoothing.ClientTime = FMath::Clamp(NetworkSmoothing.ClientTime, NetworkSmoothing.ServerTime - MinClientDeltaTime, NetworkSmoothing.ServerTime);

	// Calculate the actual difference between the new server time and the client simulation

	NetworkSmoothing.Duration = NetworkSmoothing.ServerTime - NetworkSmoothing.ClientTime;
}

void ULocomotionComponent::UpdateViewNetworkSmoothing(float DeltaTime)
{
	auto& NetworkSmoothing{ ViewState.NetworkSmoothing };

	if (!NetworkSmoothing.bEnabled ||
		NetworkSmoothing.ClientTime >= NetworkSmoothing.ServerTime ||
		NetworkSmoothing.Duration <= UE_SMALL_NUMBER)
	{
		NetworkSmoothing.InitialRotation = ReplicatedViewRotation;
		NetworkSmoothing.Rotation = ReplicatedViewRotation;
		return;
	}

	if (MovementBase.bHasRelativeRotation)
	{
		NetworkSmoothing.InitialRotation.Pitch += MovementBase.DeltaRotation.Pitch;
		NetworkSmoothing.InitialRotation.Yaw += MovementBase.DeltaRotation.Yaw;
		NetworkSmoothing.InitialRotation.Normalize();

		NetworkSmoothing.Rotation.Pitch += MovementBase.DeltaRotation.Pitch;
		NetworkSmoothing.Rotation.Yaw += MovementBase.DeltaRotation.Yaw;
		NetworkSmoothing.Rotation.Normalize();
	}

	NetworkSmoothing.ClientTime += DeltaTime;

	const auto InterpolationAmount{ ULocomotionFunctionLibrary::Clamp01(1.0f - (NetworkSmoothing.ServerTime - NetworkSmoothing.ClientTime) / NetworkSmoothing.Duration) };

	if (!FAnimWeight::IsFullWeight(InterpolationAmount))
	{
		NetworkSmoothing.Rotation = ULocomotionFunctionLibrary::LerpRotator(NetworkSmoothing.InitialRotation, ReplicatedViewRotation, InterpolationAmount);
		
	}
	else
	{
		NetworkSmoothing.ClientTime = NetworkSmoothing.ServerTime;
		NetworkSmoothing.Rotation = ReplicatedViewRotation;
	}
}

void ULocomotionComponent::SetReplicatedViewRotation(const FRotator& NewViewRotation)
{
	if (ReplicatedViewRotation != NewViewRotation)
	{
		ReplicatedViewRotation = NewViewRotation;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ReplicatedViewRotation, this);

		if (!CharacterOwner->IsReplicatingMovement() && CharacterOwner->GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetReplicatedViewRotation(ReplicatedViewRotation);
		}
	}
}

void ULocomotionComponent::OnReplicated_ReplicatedViewRotation()
{
	CorrectViewNetworkSmoothing(ReplicatedViewRotation);
}

void ULocomotionComponent::Server_SetReplicatedViewRotation_Implementation(const FRotator& NewViewRotation)
{
	SetReplicatedViewRotation(NewViewRotation);
}

#pragma endregion


#pragma region Locomotion State

void ULocomotionComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ULocomotionComponent::UpdateCharacterStateBeforeMovement()"), STAT_ULocomotionComponent_UpdateCharacterStateBeforeMovement, STATGROUP_Locomotion);

	if (!LocomotionData)
	{
		return;
	}

	UpdateVisibilityBasedAnimTickOption();

	UpdateMovementBase();

	UpdateInput(DeltaSeconds);

	UpdateLocomotionEarly();

	UpdateView(DeltaSeconds);

	UpdateLocomotion(DeltaSeconds);

	UpdateLocomotionConfigs();

	UpdateOnGroundRotation(DeltaSeconds);
	UpdateInAirRotation(DeltaSeconds);
	UpdateInWaterRotation(DeltaSeconds);

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void ULocomotionComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ULocomotionComponent::UpdateCharacterStateAfterMovement()"), STAT_ULocomotionComponent_UpdateCharacterStateAfterMovement, STATGROUP_Locomotion);

	if (!LocomotionData)
	{
		return;
	}

	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);

	UpdateLocomotionLate(DeltaSeconds);

	UpdateAnimInstanceMovement();
}

void ULocomotionComponent::ComputeFloorDist(
	const FVector& CapsuleLocation,
	float LineDistance,
	float SweepDistance,
	FFindFloorResult& OutFloorResult,
	float SweepRadius,
	const FHitResult* DownwardSweepResult) const
{
	OutFloorResult.Clear();

	auto PawnRadius{ 0.0f };
	auto PawnHalfHeight{ 0.0f };
	CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);

	auto bSkipSweep{ false };
	if (DownwardSweepResult != NULL && DownwardSweepResult->IsValidBlockingHit())
	{
		// Only if the supplied sweep was vertical and downward.

		if ((DownwardSweepResult->TraceStart.Z > DownwardSweepResult->TraceEnd.Z) &&
			(DownwardSweepResult->TraceStart - DownwardSweepResult->TraceEnd).SizeSquared2D() <= UE_KINDA_SMALL_NUMBER)
		{
			// Reject hits that are barely on the cusp of the radius of the capsule

			if (IsWithinEdgeTolerance(DownwardSweepResult->Location, DownwardSweepResult->ImpactPoint, PawnRadius))
			{
				// Don't try a redundant sweep, regardless of whether this sweep is usable.

				bSkipSweep = true;

				const auto bIsWalkable{ IsWalkable(*DownwardSweepResult) };
				const auto FloorDist{ CapsuleLocation.Z - DownwardSweepResult->Location.Z };
				OutFloorResult.SetFromSweep(*DownwardSweepResult, FloorDist, bIsWalkable);

				if (bIsWalkable)
				{
					// Use the supplied downward sweep as the floor hit result.

					return;
				}
			}
		}
	}

	// We require the sweep distance to be >= the line distance, otherwise the HitResult can't be interpreted as the sweep result.

	if (SweepDistance < LineDistance)
	{
		ensure(SweepDistance >= LineDistance);
		return;
	}

	auto bBlockingHit{ false };
	auto QueryParams{ FCollisionQueryParams(SCENE_QUERY_STAT(ComputeFloorDist), false, CharacterOwner) };
	auto ResponseParam{ FCollisionResponseParams() };
	InitCollisionParams(QueryParams, ResponseParam);
	const auto CollisionChannel{ UpdatedComponent->GetCollisionObjectType() };

	// Sweep test

	if (!bSkipSweep && SweepDistance > 0.f && SweepRadius > 0.f)
	{
		// Use a shorter height to avoid sweeps giving weird results if we start on a surface.
		// This also allows us to adjust out of penetrations.

		const auto ShrinkScale{ 0.9f };
		const auto ShrinkScaleOverlap{ 0.1f };
		auto ShrinkHeight{ (PawnHalfHeight - PawnRadius) * (1.f - ShrinkScale) };
		auto TraceDist{ SweepDistance + ShrinkHeight };
		auto CapsuleShape{ FCollisionShape::MakeCapsule(SweepRadius, PawnHalfHeight - ShrinkHeight) };

		auto Hit{ FHitResult(1.f) };
		bBlockingHit = FloorSweepTest(Hit, CapsuleLocation, CapsuleLocation + FVector(0.f, 0.f, -TraceDist), CollisionChannel, CapsuleShape, QueryParams, ResponseParam);

		/// TODO Start of custom ALS code block.

		const_cast<ThisClass*>(this)->SavePenetrationAdjustment(Hit);

		/// TODO End of custom ALS code block.

		if (bBlockingHit)
		{
			// Reject hits adjacent to us, we only care about hits on the bottom portion of our capsule.
			// Check 2D distance to impact point, reject if within a tolerance from radius.

			if (Hit.bStartPenetrating || !IsWithinEdgeTolerance(CapsuleLocation, Hit.ImpactPoint, CapsuleShape.Capsule.Radius))
			{
				// Use a capsule with a slightly smaller radius and shorter height to avoid the adjacent object.
				// Capsule must not be nearly zero or the trace will fall back to a line trace from the start point and have the wrong length.

				CapsuleShape.Capsule.Radius = FMath::Max(0.f, CapsuleShape.Capsule.Radius - SWEEP_EDGE_REJECT_DISTANCE - UE_KINDA_SMALL_NUMBER);
				if (!CapsuleShape.IsNearlyZero())
				{
					ShrinkHeight = (PawnHalfHeight - PawnRadius) * (1.f - ShrinkScaleOverlap);
					TraceDist = SweepDistance + ShrinkHeight;
					CapsuleShape.Capsule.HalfHeight = FMath::Max(PawnHalfHeight - ShrinkHeight, CapsuleShape.Capsule.Radius);
					Hit.Reset(1.f, false);

					bBlockingHit = FloorSweepTest(Hit, CapsuleLocation, CapsuleLocation + FVector(0.f, 0.f, -TraceDist), CollisionChannel, CapsuleShape, QueryParams, ResponseParam);
				}
			}

			// Reduce hit distance by ShrinkHeight because we shrank the capsule for the trace.
			// We allow negative distances here, because this allows us to pull out of penetrations.

			const auto MaxPenetrationAdjust{ FMath::Max(MAX_FLOOR_DIST, PawnRadius) };
			const auto SweepResult{ FMath::Max(-MaxPenetrationAdjust, Hit.Time * TraceDist - ShrinkHeight) };

			OutFloorResult.SetFromSweep(Hit, SweepResult, false);
			if (Hit.IsValidBlockingHit() && IsWalkable(Hit))
			{
				if (SweepResult <= SweepDistance)
				{
					// Hit within test distance.

					OutFloorResult.bWalkableFloor = true;
					return;
				}
			}
		}
	}

	// Since we require a longer sweep than line trace, we don't want to run the line trace if the sweep missed everything.
	// We do however want to try a line trace if the sweep was stuck in penetration.

	if (!OutFloorResult.bBlockingHit && !OutFloorResult.HitResult.bStartPenetrating)
	{
		OutFloorResult.FloorDist = SweepDistance;
		return;
	}

	// Line trace

	if (LineDistance > 0.f)
	{
		const auto ShrinkHeight{ PawnHalfHeight };
		const auto LineTraceStart{ FVector(CapsuleLocation) };
		const auto TraceDist{ LineDistance + ShrinkHeight };
		const auto Down{ FVector(0.f, 0.f, -TraceDist) };
		QueryParams.TraceTag = SCENE_QUERY_STAT_NAME_ONLY(FloorLineTrace);

		auto Hit{ FHitResult(1.f) };
		bBlockingHit = GetWorld()->LineTraceSingleByChannel(Hit, LineTraceStart, LineTraceStart + Down, CollisionChannel, QueryParams, ResponseParam);

		if (bBlockingHit)
		{
			if (Hit.Time > 0.f)
			{
				// Reduce hit distance by ShrinkHeight because we started the trace higher than the base.
				// We allow negative distances here, because this allows us to pull out of penetrations.

				const auto MaxPenetrationAdjust{ FMath::Max(MAX_FLOOR_DIST, PawnRadius) };
				const auto LineResult{ FMath::Max(-MaxPenetrationAdjust, Hit.Time * TraceDist - ShrinkHeight) };

				OutFloorResult.bBlockingHit = true;
				if (LineResult <= LineDistance && IsWalkable(Hit))
				{
					OutFloorResult.SetFromLineTrace(Hit, OutFloorResult.FloorDist, LineResult, true);
					return;
				}
			}
		}
	}

	// No hits were acceptable.

	OutFloorResult.bWalkableFloor = false;
}


bool ULocomotionComponent::CanAttemptJump() const
{
	if (!(IsJumpAllowed() && !LocomotionAction.IsValid()))
	{
		return false;
	}

	return true;
}


void ULocomotionComponent::ControlledCharacterMove(const FVector& InputVector, float DeltaTime)
{
	Super::ControlledCharacterMove(InputVector, DeltaTime);

	const auto* Controller{ CharacterOwner->GetController() };
	if (IsValid(Controller))
	{
		PreviousControlRotation = Controller->GetControlRotation();
	}
}

void ULocomotionComponent::PhysWalking(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CharacterOwner || (!CharacterOwner->Controller && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	if (!UpdatedComponent->IsQueryCollisionEnabled())
	{
		SetMovementMode(MOVE_Walking);
		return;
	}

	bJustTeleported = false;
	auto bCheckedFall{ false };
	auto bTriedLedgeMove{ false };
	auto remainingTime{ DeltaTime };

	// Perform the move

	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity() || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		bJustTeleported = false;
		const auto timeTick{ GetSimulationTimeStep(remainingTime, Iterations) };
		remainingTime -= timeTick;

		// Save current values

		auto* const OldBase{ GetMovementBase() };
		const auto PreviousBaseLocation{ (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector };
		const auto OldLocation{ UpdatedComponent->GetComponentLocation() };
		const auto OldFloor{ CurrentFloor };

		RestorePreAdditiveRootMotionVelocity();

		// Ensure velocity is horizontal.

		MaintainHorizontalGroundVelocity();
		const auto OldVelocity{ Velocity };
		Acceleration.Z = 0.f;

		// Apply acceleration

		if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
		{
			CalcVelocity(timeTick, GroundFriction, false, GetMaxBrakingDeceleration());
		}

		ApplyRootMotionToVelocity(timeTick);

		if (IsFalling())
		{
			// Root motion could have put us into Falling.
			// No movement has taken place this movement tick so we pass on full time/past iteration count

			StartNewPhysics(remainingTime + timeTick, Iterations - 1);
			return;
		}

		// Compute move parameters

		const auto MoveVelocity{ Velocity };
		const auto Delta{ timeTick * MoveVelocity };
		const auto bZeroDelta{ Delta.IsNearlyZero() };
		auto StepDownResult{ FStepDownResult() };

		if (bZeroDelta)
		{
			remainingTime = 0.f;
		}
		else
		{
			// try to move forward

			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			// pawn decided to jump up

			if (IsFalling())
			{
				const auto DesiredDist{ Delta.Size() };

				if (DesiredDist > UE_KINDA_SMALL_NUMBER)
				{
					const auto ActualDist{ (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D() };
					remainingTime += timeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
				}

				StartNewPhysics(remainingTime, Iterations);
				return;
			}

			//just entered water

			else if (IsSwimming()) 
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
		}

		// Update floor.
		// StepUp might have already done it for us.

		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}

		// check for ledges here

		const auto bCheckLedges{ !CanWalkOffLedges() };
		if (bCheckLedges && !CurrentFloor.IsWalkableFloor())
		{
			// calculate possible alternate movement

			const auto GravDir{ FVector(0.f, 0.f, -1.f) };
			const auto NewDelta{ bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, GravDir) };

			if (!NewDelta.IsZero())
			{
				// first revert this move

				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// avoid repeated ledge moves if the first one fails

				bTriedLedgeMove = true;

				// Try new movement direction

				Velocity = NewDelta / timeTick;
				remainingTime += timeTick;
				continue;
			}
			else
			{
				// see if it is OK to jump
				// @TODO collision : only thing that can be problem is that oldbase has world collision on

				auto bMustJump{ bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase))) };
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;

				// revert this move

				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// Validate the floor check

			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (IsMovingOnGround())
					{
						/// TODO Start of custom ALS code block.

						ApplyPendingPenetrationAdjustment();

						/// TODO End of custom ALS code block.

						// If still walking, then fall. If not, assume the user set a different mode they want to keep.

						StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}

				/// TODO Start of custom ALS code block.

				ApplyPendingPenetrationAdjustment();

				/// TODO End of custom ALS code block.

				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// The floor check failed because it started in penetration
				// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.

				auto Hit{ FHitResult(CurrentFloor.HitResult) };
				Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
				const auto RequestedAdjustment{ GetPenetrationAdjustment(Hit) };
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			// check if just entered water

			if (IsSwimming())
			{
				StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			// See if we need to start falling.

			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const auto bMustJump{ bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase))) };
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;
			}
		}


		// Allow overlap events and such to change physics state and velocity

		if (IsMovingOnGround())
		{
			// Make velocity reflect actual move

			if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				/// TODO Start of custom ALS code block.

				PrePenetrationAdjustmentVelocity = MoveVelocity;
				bPrePenetrationAdjustmentVelocityValid = true;

				/// TODO End of custom ALS code block.

				// @TODO RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
				MaintainHorizontalGroundVelocity();
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).

		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}

	if (IsMovingOnGround())
	{
		MaintainHorizontalGroundVelocity();
	}
}

void ULocomotionComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (auto Process{ CustomMovementProcesses.FindRef(CustomMovementMode) })
	{
		Process->PhysMovement(this, DeltaTime, Iterations);
		return;
	}

	if (DeltaTime < MIN_TICK_TIME)
	{
		Super::PhysCustom(DeltaTime, Iterations);
		return;
	}

	Iterations += 1;
	bJustTeleported = false;

	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = FVector::ZeroVector;
	}

	ApplyRootMotionToVelocity(DeltaTime);

	MoveUpdatedComponent(Velocity * DeltaTime, UpdatedComponent->GetComponentQuat(), false);

	Super::PhysCustom(DeltaTime, Iterations);
}

void ULocomotionComponent::PerformMovement(float DeltaTime)
{
	Super::PerformMovement(DeltaTime);

	const auto* Controller{ HasValidData() ? CharacterOwner->GetController() : nullptr };

	if (IsValid(Controller) && CharacterOwner->GetLocalRole() >= ROLE_Authority &&
		PreviousControlRotation != Controller->GetControlRotation())
	{
		if (CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy)
		{
			ServerLastTransformUpdateTimeStamp = GetPredictionData_Server_Character()->ServerAccumulatedClientTimeStamp;
		}
		else
		{
			ServerLastTransformUpdateTimeStamp = GetWorld()->GetTimeSeconds();
		}
	}
}

void ULocomotionComponent::SmoothClientPosition(float DeltaTime)
{
	auto* PredictionData{ GetPredictionData_Client_Character() };
	const auto* Mesh{ HasValidData() ? CharacterOwner->GetMesh() : nullptr };

	if (PredictionData != nullptr && IsValid(Mesh) && Mesh->IsUsingAbsoluteRotation())
	{
		const auto Rotation{ Mesh->GetComponentQuat() * CharacterOwner->GetBaseRotationOffset().Inverse() };

		PredictionData->OriginalMeshRotationOffset = Rotation;
		PredictionData->MeshRotationOffset = Rotation;
		PredictionData->MeshRotationTarget = Rotation;
	}

	Super::SmoothClientPosition(DeltaTime);
}

void ULocomotionComponent::MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAcceleration)
{
	const auto* MoveData{ static_cast<FLocomotionNetworkMoveData*>(GetCurrentNetworkMoveData()) };
	if (MoveData != nullptr)
	{
		RotationMode	= MoveData->RotationMode;
		Stance			= MoveData->Stance;
		Gait			= MoveData->Gait;

		RefreshGaitConfigs();
	}

	Super::MoveAutonomous(ClientTimeStamp, DeltaTime, CompressedFlags, NewAcceleration);

	const auto* Controller{ HasValidData() ? CharacterOwner->GetController() : nullptr };

	if (IsValid(Controller) && IsNetMode(NM_ListenServer) && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy)
	{
		const auto NewControlRotation{ Controller->GetControlRotation() };

		CorrectViewNetworkSmoothing(NewControlRotation);

		PreviousControlRotation = NewControlRotation;
	}
}


bool ULocomotionComponent::TryConsumePrePenetrationAdjustmentVelocity(FVector& OutVelocity)
{
	if (!bPrePenetrationAdjustmentVelocityValid)
	{
		OutVelocity = FVector::ZeroVector;
		return false;
	}

	OutVelocity = PrePenetrationAdjustmentVelocity;

	PrePenetrationAdjustmentVelocity = FVector::ZeroVector;
	bPrePenetrationAdjustmentVelocityValid = false;

	return true;
}

void ULocomotionComponent::SavePenetrationAdjustment(const FHitResult& Hit)
{
	if (Hit.bStartPenetrating)
	{
		PendingPenetrationAdjustment = Hit.Normal * Hit.PenetrationDepth;
	}
}

void ULocomotionComponent::ApplyPendingPenetrationAdjustment()
{
	if (PendingPenetrationAdjustment.IsNearlyZero())
	{
		return;
	}

	ResolvePenetration(ConstrainDirectionToPlane(PendingPenetrationAdjustment),
		CurrentFloor.HitResult, UpdatedComponent->GetComponentQuat());

	PendingPenetrationAdjustment = FVector::ZeroVector;
}


void ULocomotionComponent::UpdateLocomotionEarly()
{
	if (MovementBase.bHasRelativeRotation)
	{
		// Offset the rotation (and the actor's rotation) to keep it relative to the base of the movement

		LocomotionState.TargetYawAngle = FRotator3f::NormalizeAxis(LocomotionState.TargetYawAngle + MovementBase.DeltaRotation.Yaw);
		LocomotionState.ViewRelativeTargetYawAngle = FRotator3f::NormalizeAxis(LocomotionState.ViewRelativeTargetYawAngle + MovementBase.DeltaRotation.Yaw);
		LocomotionState.SmoothTargetYawAngle = FRotator3f::NormalizeAxis(LocomotionState.SmoothTargetYawAngle + MovementBase.DeltaRotation.Yaw);

		auto NewRotation{ CharacterOwner->GetActorRotation() };
		NewRotation.Pitch += MovementBase.DeltaRotation.Pitch;
		NewRotation.Yaw += MovementBase.DeltaRotation.Yaw;
		NewRotation.Normalize();

		CharacterOwner->SetActorRotation(NewRotation);
	}

	UpdateLocomotionLocationAndRotation();

	LocomotionState.PreviousVelocity = LocomotionState.Velocity;
	LocomotionState.PreviousYawAngle = UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw);
}

void ULocomotionComponent::UpdateLocomotion(float DeltaTime)
{
	LocomotionState.Velocity = Velocity;

	LocomotionState.Speed = UE_REAL_TO_FLOAT(LocomotionState.Velocity.Size2D());

	static constexpr auto HasSpeedThreshold{ 1.0f };

	LocomotionState.bHasSpeed = LocomotionState.Speed >= HasSpeedThreshold;

	if (LocomotionState.bHasSpeed)
	{
		LocomotionState.VelocityYawAngle = UE_REAL_TO_FLOAT(ULocomotionFunctionLibrary::DirectionToAngleXY(LocomotionState.Velocity));
	}

	if (LocomotionData->bRotateTowardsDesiredVelocityInVelocityDirectionRotationMode && CharacterOwner->GetLocalRole() >= ROLE_AutonomousProxy)
	{
		FVector DesiredVelocity;

		SetDesiredVelocityYawAngle(TryConsumePrePenetrationAdjustmentVelocity(DesiredVelocity) &&
			DesiredVelocity.Size2D() >= HasSpeedThreshold
			? UE_REAL_TO_FLOAT(ULocomotionFunctionLibrary::DirectionToAngleXY(DesiredVelocity))
			: LocomotionState.VelocityYawAngle);
	}

	LocomotionState.Acceleration = (LocomotionState.Velocity - LocomotionState.PreviousVelocity) / DeltaTime;

	// If there is a velocity and current acceleration, or if the velocity is above the movement speed threshold, the character is moving.

	LocomotionState.bMoving = (LocomotionState.bHasInput && LocomotionState.bHasSpeed) ||
		LocomotionState.Speed > LocomotionData->MovingSpeedThreshold;
}

void ULocomotionComponent::UpdateLocomotionLate(float DeltaTime)
{
	if (!LocomotionMode.IsValid() || LocomotionAction.IsValid())
	{
		UpdateLocomotionLocationAndRotation();
		UpdateTargetYawAngleUsingLocomotionRotation();
	}

	LocomotionState.YawSpeed = FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw - LocomotionState.PreviousYawAngle)) / DeltaTime;
}

void ULocomotionComponent::UpdateLocomotionLocationAndRotation()
{
	const auto& ActorTransform{ GetActorTransform() };

	// If network smoothing is disabled, normal actor transformations are returned.

	if (NetworkSmoothingMode == ENetworkSmoothingMode::Disabled)
	{
		LocomotionState.Location = ActorTransform.GetLocation();
		LocomotionState.RotationQuaternion = ActorTransform.GetRotation();
		LocomotionState.Rotation = CharacterOwner->GetActorRotation();
	}
	else if (CharacterOwner->GetMesh()->IsUsingAbsoluteRotation())
	{
		LocomotionState.Location = ActorTransform.TransformPosition(CharacterOwner->GetMesh()->GetRelativeLocation() - CharacterOwner->GetBaseTranslationOffset());
		LocomotionState.RotationQuaternion = ActorTransform.GetRotation();
		LocomotionState.Rotation = CharacterOwner->GetActorRotation();
	}
	else
	{
		const auto SmoothTransform{ ActorTransform *
			FTransform(CharacterOwner->GetMesh()->GetRelativeRotationCache().RotatorToQuat(CharacterOwner->GetMesh()->GetRelativeRotation()) * CharacterOwner->GetBaseRotationOffset().Inverse(),
					   CharacterOwner->GetMesh()->GetRelativeLocation() - CharacterOwner->GetBaseTranslationOffset())
		};

		LocomotionState.Location = SmoothTransform.GetLocation();
		LocomotionState.RotationQuaternion = SmoothTransform.GetRotation();
		LocomotionState.Rotation = LocomotionState.RotationQuaternion.Rotator();
	}
}

void ULocomotionComponent::SetDesiredVelocityYawAngle(float NewDesiredVelocityYawAngle)
{
	COMPARE_ASSIGN_AND_MARK_PROPERTY_DIRTY(ThisClass, DesiredVelocityYawAngle, NewDesiredVelocityYawAngle, this);
}

#pragma endregion 


#pragma region Rotation

void ULocomotionComponent::UpdateOnGroundRotation(float DeltaTime)
{
	// Suspend if LocomotionAction is not adapting or OnGround

	if (LocomotionAction.IsValid() || !IsMovingOnGround())
	{
		return;
	}

	// During RootMotion, omit updating the rotation and only update the viewpoint angle.

	if (CharacterOwner->HasAnyRootMotion())
	{
		UpdateTargetYawAngleUsingLocomotionRotation();
		return;
	}

	// If LocomotionState is Not Moving

	if (!LocomotionState.bMoving)
	{
		ApplyRotationYawSpeed(DeltaTime);

		// If Rotation mode is in the speed direction 

		if (GetRotationMode() == TAG_Status_RotationMode_VelocityDirection)
		{
			const auto TargetYawAngle{ (MovementBase.bHasRelativeLocation && !MovementBase.bHasRelativeRotation && LocomotionData->bInheritMovementBaseRotationInVelocityDirectionRotationMode)
										? FRotator3f::NormalizeAxis(LocomotionState.TargetYawAngle + MovementBase.DeltaRotation.Yaw)
										: LocomotionState.TargetYawAngle
			};

			static constexpr auto RotationInterpolationSpeed{ 12.0f };
			static constexpr auto TargetYawAngleRotationSpeed{ 800.0f };

			UpdateRotationExtraSmooth(TargetYawAngle, DeltaTime, RotationInterpolationSpeed, TargetYawAngleRotationSpeed);
		}

		// RotationMode is ViewDirection or Aiming

		else
		{
			static constexpr auto RotationInterpolationSpeed{ 20.0f };

			// If has input

			if (LocomotionState.bHasInput)
			{
				static constexpr auto TargetYawAngleRotationSpeed{ 1000.0f };

				UpdateRotationExtraSmooth(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw), DeltaTime, RotationInterpolationSpeed, TargetYawAngleRotationSpeed);
				return;
			}

			// If has not input

			else
			{
				auto ViewRelativeYawAngle{ FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw - LocomotionState.Rotation.Yaw)) };

				static constexpr auto ViewRelativeYawAngleThreshold{ 70.0f };

				// If the character is about to rotate beyond a certain angle

				if (FMath::Abs(ViewRelativeYawAngle) > ViewRelativeYawAngleThreshold)
				{
					if (ViewRelativeYawAngle > 180.0f - ULocomotionFunctionLibrary::CounterClockwiseRotationAngleThreshold)
					{
						ViewRelativeYawAngle -= 360.0f;
					}

					UpdateRotation(FRotator3f::NormalizeAxis(
						UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw + (ViewRelativeYawAngle >= 0.0f ? -ViewRelativeYawAngleThreshold : ViewRelativeYawAngleThreshold))),
							DeltaTime, RotationInterpolationSpeed);
				}

				// If the character is not trying to rotate beyond a certain angle

				else
				{
					UpdateTargetYawAngleUsingLocomotionRotation();
				}
			}
		}
	}

	// If LocomotionState is Moving

	else
	{
		// RotationMode is VelocityDirection and there is an input

		if (GetRotationMode() == TAG_Status_RotationMode_VelocityDirection &&
			(LocomotionState.bHasInput || !LocomotionState.bRotationTowardsLastInputDirectionBlocked))
		{
			LocomotionState.bRotationTowardsLastInputDirectionBlocked = false;

			const auto TargetYawAngle{ LocomotionData->bRotateTowardsDesiredVelocityInVelocityDirectionRotationMode ? DesiredVelocityYawAngle : LocomotionState.VelocityYawAngle };

			static constexpr auto TargetYawAngleRotationSpeed{ 800.0f };

			UpdateRotationExtraSmooth(TargetYawAngle, DeltaTime, CalculateRotationInterpolationSpeed(), TargetYawAngleRotationSpeed);
			return;
		}

		// RotationMode is ViewDirection or Aiming

		else
		{
			static constexpr auto RotationInterpolationSpeed{ 20.0f };
			static constexpr auto TargetYawAngleRotationSpeed{ 1000.0f };

			UpdateRotationExtraSmooth(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw), DeltaTime, RotationInterpolationSpeed, TargetYawAngleRotationSpeed);
		}
	}
}

void ULocomotionComponent::UpdateInAirRotation(float DeltaTime)
{
	// Suspend if LocomotionAction is not adapting or InAir

	if (LocomotionAction.IsValid() || !IsMovingInAir())
	{
		return;
	}

	// RotationMode is VelocityDirection

	if (GetRotationMode() == TAG_Status_RotationMode_VelocityDirection)
	{
		static constexpr auto RotationInterpolationSpeed{ 5.0f };

		// If LocomotionState is Not Moving

		if (!LocomotionState.bMoving)
		{
			UpdateTargetYawAngleUsingLocomotionRotation();
		}

		// If LocomotionState is Moving

		else
		{
			UpdateRotation(LocomotionState.VelocityYawAngle, DeltaTime, RotationInterpolationSpeed);
		}
	}

	// If RotationMode is VelocityDirection or Aiming

	else
	{
		static constexpr auto RotationInterpolationSpeed{ 15.0f };

		UpdateRotation(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw), DeltaTime, RotationInterpolationSpeed);
	}
}

void ULocomotionComponent::UpdateInWaterRotation(float DeltaTime)
{
	// Suspend if LocomotionAction is not adapting or InWater

	if (LocomotionAction.IsValid() || !IsMovingInWater())
	{
		return;
	}

	// RotationMode is VelocityDirection

	if (GetRotationMode() == TAG_Status_RotationMode_VelocityDirection)
	{
		static constexpr auto RotationInterpolationSpeed{ 5.0f };

		// If LocomotionState is Not Moving

		if (!LocomotionState.bMoving)
		{
			UpdateTargetYawAngleUsingLocomotionRotation();
		}

		// If LocomotionState is Moving

		else
		{
			UpdateRotation(LocomotionState.VelocityYawAngle, DeltaTime, RotationInterpolationSpeed);
		}
	}

	// If RotationMode is VelocityDirection or Aiming

	else
	{
		static constexpr auto RotationInterpolationSpeed{ 15.0f };

		UpdateRotation(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw), DeltaTime, RotationInterpolationSpeed);
	}
}

float ULocomotionComponent::CalculateRotationInterpolationSpeed() const
{
	static constexpr auto MaxInterpolationSpeedMultiplier{ 3.0f };
	static constexpr auto ReferenceViewYawSpeed{ 300.0f };

	return RotationInterpSpeed * ULocomotionFunctionLibrary::LerpClamped(1.0f, MaxInterpolationSpeedMultiplier, ViewState.YawSpeed / ReferenceViewYawSpeed);
}

void ULocomotionComponent::UpdateRotation(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed)
{
	UpdateTargetYawAngle(TargetYawAngle);

	auto NewRotation{ CharacterOwner->GetActorRotation() };

	NewRotation.Yaw = ULocomotionFunctionLibrary::ExponentialDecayAngle(UE_REAL_TO_FLOAT(FRotator::NormalizeAxis(NewRotation.Yaw)), TargetYawAngle, DeltaTime, RotationInterpolationSpeed);

	CharacterOwner->SetActorRotation(NewRotation);

	UpdateLocomotionLocationAndRotation();
}

void ULocomotionComponent::UpdateRotationExtraSmooth(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed, float TargetYawAngleRotationSpeed)
{
	LocomotionState.TargetYawAngle = TargetYawAngle;

	UpdateViewRelativeTargetYawAngle();

	LocomotionState.SmoothTargetYawAngle = ULocomotionFunctionLibrary::InterpolateAngleConstant(LocomotionState.SmoothTargetYawAngle, TargetYawAngle, DeltaTime, TargetYawAngleRotationSpeed);

	auto NewRotation{ CharacterOwner->GetActorRotation() };

	NewRotation.Yaw = ULocomotionFunctionLibrary::ExponentialDecayAngle(UE_REAL_TO_FLOAT(FRotator::NormalizeAxis(NewRotation.Yaw)), LocomotionState.SmoothTargetYawAngle, DeltaTime, RotationInterpolationSpeed);

	CharacterOwner->SetActorRotation(NewRotation);

	UpdateLocomotionLocationAndRotation();
}

void ULocomotionComponent::UpdateRotationInstant(float TargetYawAngle, ETeleportType Teleport)
{
	UpdateTargetYawAngle(TargetYawAngle);

	auto NewRotation{ CharacterOwner->GetActorRotation() };

	NewRotation.Yaw = TargetYawAngle;

	CharacterOwner->SetActorRotation(NewRotation, Teleport);

	UpdateLocomotionLocationAndRotation();
}

void ULocomotionComponent::UpdateTargetYawAngleUsingLocomotionRotation()
{
	UpdateTargetYawAngle(UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw));
}

void ULocomotionComponent::UpdateTargetYawAngle(float TargetYawAngle)
{
	LocomotionState.TargetYawAngle = TargetYawAngle;

	UpdateViewRelativeTargetYawAngle();

	LocomotionState.SmoothTargetYawAngle = TargetYawAngle;
}

void ULocomotionComponent::UpdateViewRelativeTargetYawAngle()
{
	LocomotionState.ViewRelativeTargetYawAngle = FRotator3f::NormalizeAxis(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw - LocomotionState.TargetYawAngle));
}

void ULocomotionComponent::ApplyRotationYawSpeed(float DeltaTime)
{
	const auto DeltaYawAngle{ CharacterOwner->GetMesh()->GetAnimInstance()->GetCurveValue(ULocomotionGeneralNameStatics::RotationYawSpeedCurveName()) * DeltaTime };
	
	if (FMath::Abs(DeltaYawAngle) > UE_SMALL_NUMBER)
	{
		auto NewRotation{ CharacterOwner->GetActorRotation() };

		NewRotation.Yaw += DeltaYawAngle;

		CharacterOwner->SetActorRotation(NewRotation);

		UpdateLocomotionLocationAndRotation();
		UpdateTargetYawAngleUsingLocomotionRotation();
	}
}

#pragma endregion


#pragma region Utilities

ULocomotionComponent* ULocomotionComponent::FindLocomotionComponent(const ACharacter* Character)
{
	return (Character ? Character->FindComponentByClass<ULocomotionComponent>() : nullptr);
}

#pragma endregion

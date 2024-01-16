// Copyright (C) 2024 owoDra

#include "LocomotionCharacter.h"

#include "CharacterAnimInstance.h"
#include "State/ViewState.h"
#include "LocomotionFunctionLibrary.h"
#include "LocomotionComponent.h"

#include "Net/UnrealNetwork.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LocomotionCharacter)


ALocomotionCharacter::ALocomotionCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<ULocomotionComponent>(ACharacter::CharacterMovementComponentName))
{
	// Do not apply Controller rotation.

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Setup Network settings

	NetCullDistanceSquared = 900000000.0f;
	bClientCheckEncroachmentOnNetUpdate = true; // Required to update bSimGravityDisabled.

	// Setup Capsule

	auto* CapsuleComp{ GetCapsuleComponent() };
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);

	// Setup Mesh

	auto* MeshComp{ GetMesh() };
	check(MeshComp);
	MeshComp->SetRelativeRotation_Direct(FRotator(0.0f, -90.0f, 0.0f));

	// Setup eye height

	BaseEyeHeight = 80.0f;
	CrouchedEyeHeight = 50.0f;

	// Cache GCLA Character Movement Component

	LocomotionComponent= Cast<ULocomotionComponent>(GetCharacterMovement());

	// Setup property flags of parent class

#if WITH_EDITOR
	StaticClass()->FindPropertyByName(FName{ TEXTVIEW("Mesh") })->SetPropertyFlags(CPF_DisableEditOnInstance);
	StaticClass()->FindPropertyByName(FName{ TEXTVIEW("CapsuleComponent") })->SetPropertyFlags(CPF_DisableEditOnInstance);
	StaticClass()->FindPropertyByName(FName{ TEXTVIEW("CharacterMovement") })->SetPropertyFlags(CPF_DisableEditOnInstance);
#endif
}

#if WITH_EDITOR
bool ALocomotionCharacter::CanEditChange(const FProperty* Property) const
{
	return Super::CanEditChange(Property) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, bUseControllerRotationPitch) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, bUseControllerRotationYaw) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, bUseControllerRotationRoll);
}
#endif


#pragma region Replication

void ALocomotionCharacter::PostNetReceiveLocationAndRotation()
{
	// This function is only called on the simulated proxy, so there is no need to check roles here

	const auto PreviousLocation{ GetActorLocation() };

	// Character itself has full control over rotation, ignoring server-replicated rotation

	GetReplicatedMovement_Mutable().Rotation = GetActorRotation();

	Super::PostNetReceiveLocationAndRotation();

	// Detect teleportation of simulated proxies

	auto bTeleported{ static_cast<bool>(bSimGravityDisabled) };

	if (!bTeleported && !ReplicatedBasedMovement.HasRelativeLocation())
	{
		const auto NewLocation{ FRepMovement::RebaseOntoLocalOrigin(GetReplicatedMovement().Location, this) };

		bTeleported |= FVector::DistSquared(PreviousLocation, NewLocation) > ULocomotionFunctionLibrary::TeleportDistanceThresholdSquared;
	}

	if (bTeleported)
	{
		if (auto MainMesh{ ICharacterMeshAccessorInterface::Execute_GetMainMesh(this) })
		{
			if (auto* AnimIns{ Cast<UCharacterAnimInstance>(MainMesh->GetAnimInstance()) })
			{
				AnimIns->MarkTeleported();
			}
		}
	}
}

void ALocomotionCharacter::OnRep_ReplicatedBasedMovement()
{
	// This function is only called on the simulated proxy, so there is no need to check roles here

	const auto PreviousLocation{ GetActorLocation() };

	// Character itself has full control over rotation, ignoring server-replicated rotation

	if (ReplicatedBasedMovement.HasRelativeRotation())
	{
		FVector MovementBaseLocation;
		FQuat MovementBaseRotation;

		MovementBaseUtility::GetMovementBaseTransform(ReplicatedBasedMovement.MovementBase, ReplicatedBasedMovement.BoneName, MovementBaseLocation, MovementBaseRotation);
		ReplicatedBasedMovement.Rotation = (MovementBaseRotation.Inverse() * GetActorQuat()).Rotator();
	}
	else
	{
		ReplicatedBasedMovement.Rotation = GetActorRotation();
	}

	Super::OnRep_ReplicatedBasedMovement();

	// Detect teleportation of simulated proxies

	auto bTeleported{ static_cast<bool>(bSimGravityDisabled) };

	if (!bTeleported && BasedMovement.HasRelativeLocation())
	{
		const auto NewLocation{ LocomotionComponent->OldBaseLocation + LocomotionComponent->OldBaseQuat.RotateVector(BasedMovement.Location) };

		bTeleported |= FVector::DistSquared(PreviousLocation, NewLocation) > ULocomotionFunctionLibrary::TeleportDistanceThresholdSquared;
	}

	if (bTeleported)
	{
		if (auto MainMesh{ ICharacterMeshAccessorInterface::Execute_GetMainMesh(this) })
		{
			if (auto* AnimIns{ Cast<UCharacterAnimInstance>(MainMesh->GetAnimInstance()) })
			{
				AnimIns->MarkTeleported();
			}
		}
	}
}

#pragma endregion


#pragma region Initialize and Uninitialize

void ALocomotionCharacter::PostInitializeComponents()
{
	auto Meshes{ ICharacterMeshAccessorInterface::Execute_GetMeshes(this) };

	for (const auto& Each : Meshes)
	{
		Each->AddTickPrerequisiteActor(this);
	}

	Super::PostInitializeComponents();
}

void ALocomotionCharacter::Reset()
{
	DisableMovementAndCollision();

	K2_OnReset();

	UninitAndDestroy();
}


void ALocomotionCharacter::DisableMovementAndCollision()
{
	if (Controller)
	{
		Controller->SetIgnoreMoveInput(true);
	}

	auto* CapsuleComp{ GetCapsuleComponent() };
	check(CapsuleComp);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	check(LocomotionComponent);
	LocomotionComponent->StopMovementImmediately();
	LocomotionComponent->DisableMovement();
}

void ALocomotionCharacter::UninitAndDestroy()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		DetachFromControllerPendingDestroy();
		SetLifeSpan(0.1f);
	}

	SetActorHiddenInGame(true);
}


void ALocomotionCharacter::BeginPlay()
{
	ensureMsgf(!bUseControllerRotationPitch && !bUseControllerRotationYaw && !bUseControllerRotationRoll, TEXT("bUseControllerRotationXXX must be turned off!"));

	Super::BeginPlay();

	if (GetLocalRole() >= ROLE_AutonomousProxy)
	{
		GetCapsuleComponent()->TransformUpdated.AddWeakLambda(this, 
			[this](USceneComponent*, const EUpdateTransformFlags, const ETeleportType TeleportType)
			{
				if (TeleportType != ETeleportType::None)
				{
					if (auto MainMesh{ ICharacterMeshAccessorInterface::Execute_GetMainMesh(this) })
					{
						if (auto* AnimIns{ Cast<UCharacterAnimInstance>(MainMesh->GetAnimInstance()) })
						{
							AnimIns->MarkTeleported();
						}
					}
				}
			}
		);
	}

	LocomotionComponent->UpdateUsingAbsoluteRotation();
	LocomotionComponent->UpdateVisibilityBasedAnimTickOption();
}

void ALocomotionCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	LocomotionComponent->UpdateUsingAbsoluteRotation();
	LocomotionComponent->UpdateVisibilityBasedAnimTickOption();
}

#pragma endregion


#pragma region Stance

bool ALocomotionCharacter::CanCrouch() const
{
	return GetCharacterMovement() && GetCharacterMovement()->CanEverCrouch() && GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics();
}

void ALocomotionCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	auto* PredictionData{ GetCharacterMovement()->GetPredictionData_Client_Character() };

	if (PredictionData != nullptr && GetLocalRole() <= ROLE_SimulatedProxy &&
		ScaledHalfHeightAdjust > 0.0f && IsPlayingNetworkedRootMotionMontage())
	{
		// The following code essentially undoes the changes made at the end of UCharacterMovementComponent::Crouch().
		// This is because crouching during playback of the root motion montage literally breaks the network smoothing.
		// This is because network smoothing is literally interrupted if you crouch while the root motion montage is playing.

		// @TODO Please confirm the need for this fix in a future engine version.

		PredictionData->MeshTranslationOffset.Z += ScaledHalfHeightAdjust;
		PredictionData->OriginalMeshTranslationOffset = PredictionData->MeshTranslationOffset;
	}

	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void ALocomotionCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	auto* PredictionData{ GetCharacterMovement()->GetPredictionData_Client_Character() };

	if (PredictionData != nullptr && GetLocalRole() <= ROLE_SimulatedProxy &&
		ScaledHalfHeightAdjust > 0.0f && IsPlayingNetworkedRootMotionMontage())
	{
		// Same fix as in AAlsCharacter::OnStartCrouch().

		PredictionData->MeshTranslationOffset.Z -= ScaledHalfHeightAdjust;
		PredictionData->OriginalMeshTranslationOffset = PredictionData->MeshTranslationOffset;
	}

	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

#pragma endregion


#pragma region View

FRotator ALocomotionCharacter::GetViewRotation() const
{
	return LocomotionComponent->GetViewState().Rotation;
}

FRotator ALocomotionCharacter::GetViewRotationSuperClass() const
{
	if (Controller != nullptr)
	{
		return Controller->GetControlRotation();
	}
	else if (GetLocalRole() < ROLE_Authority)
	{
		// check if being spectated

		for (auto Iterator{ GetWorld()->GetPlayerControllerIterator() }; Iterator; ++Iterator)
		{
			auto* PlayerController{ Iterator->Get() };
			if (PlayerController &&
				PlayerController->PlayerCameraManager &&
				PlayerController->PlayerCameraManager->GetViewTargetPawn() == this)
			{
				return PlayerController->BlendedTargetViewRotation;
			}
		}
	}

	return GetActorRotation();
}

#pragma endregion


#pragma region Jump

bool ALocomotionCharacter::CanJumpInternal_Implementation() const
{
	return JumpIsAllowedInternal();
}

#pragma endregion


#pragma region Mesh Accessor

TArray<USkeletalMeshComponent*> ALocomotionCharacter::GetMeshes_Implementation() const
{
	TArray<USkeletalMeshComponent*> Result;

	Result.Emplace(GetMesh());

	return Result;
}

USkeletalMeshComponent* ALocomotionCharacter::GetMainMesh_Implementation() const
{
	return GetMesh();
}

USkeletalMeshComponent* ALocomotionCharacter::GetMeshByTag_Implementation(const FGameplayTag& Tag) const
{
	return GetMesh();
}

#pragma endregion

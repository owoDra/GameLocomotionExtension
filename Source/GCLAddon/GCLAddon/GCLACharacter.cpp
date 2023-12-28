// Copyright (C) 2023 owoDra

#include "GCLACharacter.h"

#include "CharacterLocomotionDeveloperSettings.h"
#include "Animation/CharacterAnimInstance.h"
#include "Movement/MovementMathLibrary.h"
#include "Movement/State/ViewState.h"
#include "GCLACharacterMovementComponent.h"

#include "CharacterDataComponent.h"

#include "Net/UnrealNetwork.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCLACharacter)


AGCLACharacter::AGCLACharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGCLACharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Do not apply Controller rotation.

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Setup Network settings

	NetCullDistanceSquared = 900000000.0f;
	bClientCheckEncroachmentOnNetUpdate = true; // Required to update bSimGravityDisabled.

	// Setup Capsule

	const auto* DevSettings{ GetDefault<UCharacterLocomotionDeveloperSettings>() };

	auto* CapsuleComp{ GetCapsuleComponent() };
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(DevSettings->CapsuleCollisionProfileName);

	// Setup Mesh

	auto* MeshComp{ GetMesh() };
	check(MeshComp);
	MeshComp->SetRelativeRotation_Direct(FRotator(0.0f, -90.0f, 0.0f));
	MeshComp->SetCollisionProfileName(DevSettings->MeshCollisionProfileName);

	// Setup eye height

	BaseEyeHeight = 80.0f;
	CrouchedEyeHeight = 50.0f;

	// Setup Character Data Component

	CharacterDataComponent = CreateDefaultSubobject<UCharacterDataComponent>(TEXT("CharacterDataComponent"));

	// Cache GCLA Character Movement Component

	GCLACharacterMovementComponent = Cast<UGCLACharacterMovementComponent>(GetCharacterMovement());

	// Setup property flags of parent class

#if WITH_EDITOR
	StaticClass()->FindPropertyByName(FName{ TEXTVIEW("Mesh") })->SetPropertyFlags(CPF_DisableEditOnInstance);
	StaticClass()->FindPropertyByName(FName{ TEXTVIEW("CapsuleComponent") })->SetPropertyFlags(CPF_DisableEditOnInstance);
	StaticClass()->FindPropertyByName(FName{ TEXTVIEW("CharacterMovement") })->SetPropertyFlags(CPF_DisableEditOnInstance);
#endif
}

#if WITH_EDITOR
bool AGCLACharacter::CanEditChange(const FProperty* Property) const
{
	return Super::CanEditChange(Property) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, bUseControllerRotationPitch) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, bUseControllerRotationYaw) &&
		Property->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, bUseControllerRotationRoll);
}
#endif


#pragma region Replication

void AGCLACharacter::PostNetReceiveLocationAndRotation()
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

		bTeleported |= FVector::DistSquared(PreviousLocation, NewLocation) > UMovementMathLibrary::TeleportDistanceThresholdSquared;
	}

	if (bTeleported)
	{
		if (auto MainMesh{ ICharacterMeshAccessorInterface::Execute_GetMainMesh(this) })
		{
			if (auto* AnimIns{ Cast<UCharacterAnimInstance>(MainMesh->GetAnimInstance()) })
			{
				AnimIns->MarkPendingUpdate();
			}
		}
	}
}

void AGCLACharacter::OnRep_ReplicatedBasedMovement()
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
		const auto NewLocation{ GCLACharacterMovementComponent->OldBaseLocation + GCLACharacterMovementComponent->OldBaseQuat.RotateVector(BasedMovement.Location) };

		bTeleported |= FVector::DistSquared(PreviousLocation, NewLocation) > UMovementMathLibrary::TeleportDistanceThresholdSquared;
	}

	if (bTeleported)
	{
		if (auto MainMesh{ ICharacterMeshAccessorInterface::Execute_GetMainMesh(this) })
		{
			if (auto* AnimIns{ Cast<UCharacterAnimInstance>(MainMesh->GetAnimInstance()) })
			{
				AnimIns->MarkPendingUpdate();
			}
		}
	}
}

#pragma endregion


#pragma region Initialize and Uninitialize

void AGCLACharacter::PostInitializeComponents()
{
	auto Meshes{ ICharacterMeshAccessorInterface::Execute_GetMeshes(this) };

	for (const auto& Each : Meshes)
	{
		Each->AddTickPrerequisiteActor(this);
	}

	Super::PostInitializeComponents();
}

void AGCLACharacter::Reset()
{
	DisableMovementAndCollision();

	K2_OnReset();

	UninitAndDestroy();
}


void AGCLACharacter::DisableMovementAndCollision()
{
	if (Controller)
	{
		Controller->SetIgnoreMoveInput(true);
	}

	auto* CapsuleComp{ GetCapsuleComponent() };
	check(CapsuleComp);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	check(GCLACharacterMovementComponent);
	GCLACharacterMovementComponent->StopMovementImmediately();
	GCLACharacterMovementComponent->DisableMovement();
}

void AGCLACharacter::UninitAndDestroy()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		DetachFromControllerPendingDestroy();
		SetLifeSpan(0.1f);
	}

	SetActorHiddenInGame(true);
}


void AGCLACharacter::BeginPlay()
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
							AnimIns->MarkPendingUpdate();
						}
					}
				}
			}
		);
	}

	GCLACharacterMovementComponent->UpdateUsingAbsoluteRotation();
	GCLACharacterMovementComponent->UpdateVisibilityBasedAnimTickOption();
}

void AGCLACharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	GCLACharacterMovementComponent->UpdateUsingAbsoluteRotation();
	GCLACharacterMovementComponent->UpdateVisibilityBasedAnimTickOption();
}

#pragma endregion


#pragma region Stance

bool AGCLACharacter::CanCrouch() const
{
	return GetCharacterMovement() && GetCharacterMovement()->CanEverCrouch() && GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics();
}

void AGCLACharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
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

void AGCLACharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
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

FRotator AGCLACharacter::GetViewRotation() const
{
	return GCLACharacterMovementComponent->GetViewState().Rotation;
}

FRotator AGCLACharacter::GetViewRotationSuperClass() const
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

bool AGCLACharacter::CanJumpInternal_Implementation() const
{
	return JumpIsAllowedInternal();
}

#pragma endregion


#pragma region Mesh Accessor

TArray<USkeletalMeshComponent*> AGCLACharacter::GetMeshes_Implementation() const
{
	TArray<USkeletalMeshComponent*> Result;

	Result.Emplace(GetMesh());

	return Result;
}

USkeletalMeshComponent* AGCLACharacter::GetMainMesh_Implementation() const
{
	return GetMesh();
}

USkeletalMeshComponent* AGCLACharacter::GetMeshByTag_Implementation(const FGameplayTag& Tag) const
{
	return GetMesh();
}

#pragma endregion

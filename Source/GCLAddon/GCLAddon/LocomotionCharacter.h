// Copyright (C) 2024 owoDra

#pragma once

#include "Actor/GFCCharacter.h"
#include "CharacterMeshAccessorInterface.h"

#include "LocomotionCharacter.generated.h"

class ULocomotionComponent;
class UCharacterDataComponent;


/**
 * Character classes with extended character movement related features
 */
UCLASS()
class GCLADDON_API ALocomotionCharacter 
	: public AGFCCharacter
	, public ICharacterMeshAccessorInterface
{
	GENERATED_BODY()
public:
	explicit ALocomotionCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UCharacterDataComponent> CharacterDataComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<ULocomotionComponent> LocomotionComponent;


	////////////////////////////////////////////////
	// Replication
public:
	virtual void PostNetReceiveLocationAndRotation() override;
	virtual void OnRep_ReplicatedBasedMovement() override;


	////////////////////////////////////////////////
	// Initialize and Uninitialize
public:
	virtual void PostInitializeComponents() override;

	virtual void Reset() override;

protected:
	void DisableMovementAndCollision();
	void UninitAndDestroy();

protected:
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;


	////////////////////////////////////////////////
	// Stance
public:
	virtual bool CanCrouch() const override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;


	////////////////////////////////////////////////
	// View
public:
	/**
	 * Get the current ViewState.Rotation
	 */
	virtual FRotator GetViewRotation() const override;

	/**
	 * Call parent function GetViewRotation()
	 * 
	 * Tips:
	 *  Overridden GetViewRotation() to replace an entirely different data acquisition
	 *  Used to access the original GetViewRotation() function.
	 */
	virtual FRotator GetViewRotationSuperClass() const;


	////////////////////////////////////////////////
	// Jump
public:
	virtual bool CanJumpInternal_Implementation() const;


	////////////////////////////////////////////////
	// Rotation
public:
	/**
	 * Processing of this function is no longer used.
	 */
	virtual void FaceRotation(FRotator Rotation, float DeltaTime) override final {}
	

	////////////////////////////////////////////////
	// Mesh Accessor
public:
	TArray<USkeletalMeshComponent*> GetMeshes_Implementation() const override;
	USkeletalMeshComponent* GetMainMesh_Implementation() const override;
	USkeletalMeshComponent* GetMeshByTag_Implementation(const FGameplayTag& Tag) const override;

};

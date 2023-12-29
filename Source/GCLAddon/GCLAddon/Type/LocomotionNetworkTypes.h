// Copyright (C) 2023 owoDra

#pragma once

#include "GameFramework/CharacterMovementComponent.h"

#include "GameplayTagContainer.h"


/**
 * FLocomotionNetworkMoveData
 */
class FLocomotionNetworkMoveData : public FCharacterNetworkMoveData
{
private:
	typedef FCharacterNetworkMoveData Super;

public:
	FLocomotionNetworkMoveData();

public:
	FGameplayTag RotationMode;
	FGameplayTag Stance;
	FGameplayTag Gait;

public:
	virtual void ClientFillNetworkMoveData(const FSavedMove_Character& Move, ENetworkMoveType MoveType) override;
	virtual bool Serialize(UCharacterMovementComponent& Movement, FArchive& Archive, UPackageMap* Map, ENetworkMoveType MoveType) override;

};


/**
 * FLocomotionNetworkMoveDataContainer
 */
class FLocomotionNetworkMoveDataContainer : public FCharacterNetworkMoveDataContainer
{
public:
	FLocomotionNetworkMoveDataContainer();

public:
	FLocomotionNetworkMoveData MoveData[3];

};


/**
 * FLocomotionSavedMove
 */
class FLocomotionSavedMove : public FSavedMove_Character
{
private:
	typedef FSavedMove_Character Super;

public:
	FLocomotionSavedMove();

public:
	FGameplayTag RotationMode;
	FGameplayTag Stance;
	FGameplayTag Gait;

public:
	virtual void Clear() override;
	virtual void SetMoveFor(ACharacter* Character, float NewDeltaTime, const FVector& NewAcceleration, FNetworkPredictionData_Client_Character& PredictionData) override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* Character, float MaxDelta) const override;
	virtual void CombineWith(const FSavedMove_Character* PreviousMove, ACharacter* Character, APlayerController* Player, const FVector& PreviousStartLocation) override;
	virtual void PrepMoveFor(ACharacter* Character) override;

};


/**
 * FLocomotionNetworkPredictionData
 */
class FLocomotionNetworkPredictionData : public FNetworkPredictionData_Client_Character
{
private:
	typedef FNetworkPredictionData_Client_Character Super;

public:
	explicit FLocomotionNetworkPredictionData(const UCharacterMovementComponent& Movement);

	virtual FSavedMovePtr AllocateNewMove() override;

};

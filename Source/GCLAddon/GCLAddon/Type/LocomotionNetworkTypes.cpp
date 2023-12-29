// Copyright (C) 2023 owoDra

#include "LocomotionNetworkTypes.h"

#include "LocomotionComponent.h"
#include "GameplayTag/GCLATags_Status.h"

#include "GameFramework/Character.h"


/////////////////////////////////////////////////////////////////////////
// FLocomotionNetworkMoveData

#pragma region FLocomotionNetworkMoveData

FLocomotionNetworkMoveData::FLocomotionNetworkMoveData()
{
	RotationMode	= TAG_Status_RotationMode_ViewDirection;
	Stance			= TAG_Status_Stance_Standing;
	Gait			= TAG_Status_Gait_Walking;
}

void FLocomotionNetworkMoveData::ClientFillNetworkMoveData(const FSavedMove_Character& Move, ENetworkMoveType MoveType)
{
	Super::ClientFillNetworkMoveData(Move, MoveType);

	const auto& SavedMove{ static_cast<const FLocomotionSavedMove&>(Move) };

	RotationMode	= SavedMove.RotationMode;
	Stance			= SavedMove.Stance;
	Gait			= SavedMove.Gait;
}

bool FLocomotionNetworkMoveData::Serialize(UCharacterMovementComponent& Movement, FArchive& Archive, UPackageMap* Map, const ENetworkMoveType MoveType) 
{
	Super::Serialize(Movement, Archive, Map, MoveType);

	NetSerializeOptionalValue(Archive.IsSaving(), Archive, RotationMode	, TAG_Status_RotationMode_ViewDirection.GetTag(), Map);
	NetSerializeOptionalValue(Archive.IsSaving(), Archive, Stance		, TAG_Status_Stance_Standing.GetTag()			, Map);
	NetSerializeOptionalValue(Archive.IsSaving(), Archive, Gait			, TAG_Status_Gait_Walking.GetTag()				, Map);

	return !Archive.IsError();
}

#pragma endregion


/////////////////////////////////////////////////////////////////////////
// FLocomotionNetworkMoveDataContainer

#pragma region FLocomotionNetworkMoveDataContainer

FLocomotionNetworkMoveDataContainer::FLocomotionNetworkMoveDataContainer()
{
	NewMoveData		= &MoveData[0];
	PendingMoveData = &MoveData[1];
	OldMoveData		= &MoveData[2];
}

#pragma endregion


/////////////////////////////////////////////////////////////////////////
// FLocomotionSavedMove

#pragma region FLocomotionSavedMove

FLocomotionSavedMove::FLocomotionSavedMove()
{
	RotationMode	= TAG_Status_RotationMode_ViewDirection;
	Stance			= TAG_Status_Stance_Standing;
	Gait			= TAG_Status_Gait_Walking;
}

void FLocomotionSavedMove::Clear()
{
	Super::Clear();

	RotationMode	= TAG_Status_RotationMode_ViewDirection;
	Stance			= TAG_Status_Stance_Standing;
	Gait			= TAG_Status_Gait_Walking;
}

void FLocomotionSavedMove::SetMoveFor(ACharacter* Character, float NewDeltaTime, const FVector& NewAcceleration, FNetworkPredictionData_Client_Character& PredictionData)
{
	Super::SetMoveFor(Character, NewDeltaTime, NewAcceleration, PredictionData);

	if (const auto* Movement{ Cast<ULocomotionComponent>(Character->GetCharacterMovement()) })
	{
		RotationMode	= Movement->RotationMode;
		Stance			= Movement->Stance;
		Gait			= Movement->Gait;
	}
}

bool FLocomotionSavedMove::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* Character, float MaxDelta) const
{
	const auto* NewMove{ static_cast<FLocomotionSavedMove*>(NewMovePtr.Get()) };

	return (RotationMode == NewMove->RotationMode) && (Stance == NewMove->Stance) && (Gait == NewMove->Gait) && Super::CanCombineWith(NewMovePtr, Character, MaxDelta);
}

void FLocomotionSavedMove::CombineWith(const FSavedMove_Character* PreviousMove, ACharacter* Character, APlayerController* Player, const FVector& PreviousStartLocation)
{
	const auto OriginalRotation{ PreviousMove->StartRotation };
	const auto OriginalRelativeRotation{ PreviousMove->StartAttachRelativeRotation };

	const auto* UpdatedComponent{ Character->GetCharacterMovement()->UpdatedComponent.Get() };

	const_cast<FSavedMove_Character*>(PreviousMove)->StartRotation				 = UpdatedComponent->GetComponentRotation();
	const_cast<FSavedMove_Character*>(PreviousMove)->StartAttachRelativeRotation = UpdatedComponent->GetRelativeRotation();

	Super::CombineWith(PreviousMove, Character, Player, PreviousStartLocation);

	const_cast<FSavedMove_Character*>(PreviousMove)->StartRotation				 = OriginalRotation;
	const_cast<FSavedMove_Character*>(PreviousMove)->StartAttachRelativeRotation = OriginalRelativeRotation;
}

void FLocomotionSavedMove::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	if (auto* Movement{ Cast<ULocomotionComponent>(Character->GetCharacterMovement()) })
	{
		Movement->RotationMode	= RotationMode;
		Movement->Stance		= Stance;
		Movement->Gait			= Gait;

		Movement->RefreshGaitConfigs();
	}
}

#pragma endregion


/////////////////////////////////////////////////////////////////////////
// FLocomotionNetworkPredictionData

#pragma region FLocomotionNetworkPredictionData

FLocomotionNetworkPredictionData::FLocomotionNetworkPredictionData(const UCharacterMovementComponent& Movement)
	: Super(Movement)
{
}

FSavedMovePtr FLocomotionNetworkPredictionData::AllocateNewMove()
{
	return MakeShared<FLocomotionSavedMove>();
}

#pragma endregion

// Copyright (C) 2024 owoDra

#pragma once

#include "Animation/AnimNotifies/AnimNotifyState.h"

#include "GameplayTagContainer.h"

#include "AnimNotifyState_SetLocomotionAction.generated.h"


/**
 * AnimNotifyState class to override the character's LocomotionAction during animation playback.
 */
UCLASS(BlueprintType, meta = (DisplayName = "ANS Set Locomotion Action"))
class UAnimNotifyState_SetLocomotionAction : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UAnimNotifyState_SetLocomotionAction(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LocomotionAction", meta = (Categories = "Status.LocomotionAction"))
	FGameplayTag LocomotionAction;

public:
	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration, 
		const FAnimNotifyEventReference& EventReference) override;
	
	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

};
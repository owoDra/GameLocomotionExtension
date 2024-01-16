// Copyright (C) 2024 owoDra

#include "AnimNotifyState_SetLocomotionAction.h"

#include "LocomotionComponent.h"
#include "LocomotionCharacter.h"

#include "Components/SkeletalMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimNotifyState_SetLocomotionAction)


UAnimNotifyState_SetLocomotionAction::UAnimNotifyState_SetLocomotionAction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UAnimNotifyState_SetLocomotionAction::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	auto* LocomotionCharacter{ MeshComp->GetOwner<ALocomotionCharacter>() };
	auto* LocomotionComponent{ LocomotionCharacter ? Cast<ULocomotionComponent>(LocomotionCharacter->GetCharacterMovement()) : nullptr };

	if (LocomotionComponent)
	{
		LocomotionComponent->SetLocomotionAction(LocomotionAction);
	}
}

void UAnimNotifyState_SetLocomotionAction::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	auto* LocomotionCharacter{ MeshComp->GetOwner<ALocomotionCharacter>() };
	auto* LocomotionComponent{ LocomotionCharacter ? Cast<ULocomotionComponent>(LocomotionCharacter->GetCharacterMovement()) : nullptr };

	if (LocomotionComponent)
	{
		LocomotionComponent->SetLocomotionAction(FGameplayTag::EmptyTag);
	}
}

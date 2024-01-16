// Copyright (C) 2024 owoDra

#include "AnimNotify_PlayPairMontage.h"

#include "LocomotionCharacter.h"

#include "Character/CharacterMeshAccessorInterface.h"

#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimNotify_PlayPairMontage)


UAnimNotify_PlayPairMontage::UAnimNotify_PlayPairMontage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UAnimNotify_PlayPairMontage::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	auto* LocomotionCharacter{ MeshComp->GetOwner<ALocomotionCharacter>() };
	auto* Mesh{ LocomotionCharacter ? ICharacterMeshAccessorInterface::Execute_GetMeshByTag(LocomotionCharacter, MeshTag) : nullptr };

	if (auto* FollowerAnimIns{ Mesh ? Mesh->GetAnimInstance() : nullptr })
	{
		FollowerAnimIns->Montage_Play(Montage);

		auto* OtherAnimIns{ MeshComp->GetAnimInstance() };

		auto* MontageLeader{ Cast<UAnimMontage>(Animation) };

		FollowerAnimIns->MontageSync_Follow(Montage, OtherAnimIns, MontageLeader);
	}
}

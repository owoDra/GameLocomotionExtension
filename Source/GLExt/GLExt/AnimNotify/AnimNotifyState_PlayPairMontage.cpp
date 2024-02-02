// Copyright (C) 2024 owoDra

#include "AnimNotifyState_PlayPairMontage.h"

#include "LocomotionCharacter.h"

#include "Character/CharacterMeshAccessorInterface.h"

#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimNotifyState_PlayPairMontage)


UAnimNotifyState_PlayPairMontage::UAnimNotifyState_PlayPairMontage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


FString UAnimNotifyState_PlayPairMontage::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("[%s] Play Pair Montage: %s(%s)")
		, bShouldSync ? TEXT("SYNC") : TEXT("ASYNC")
		, *GetNameSafe(Montage)
		, *MeshTag.GetTagName().ToString());
}

void UAnimNotifyState_PlayPairMontage::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	auto* LocomotionCharacter{ MeshComp->GetOwner<ALocomotionCharacter>() };
	auto* Mesh{ LocomotionCharacter ? ICharacterMeshAccessorInterface::Execute_GetMeshByTag(LocomotionCharacter, MeshTag) : nullptr };

	if (auto* FollowerAnimIns{ Mesh ? Mesh->GetAnimInstance() : nullptr })
	{
		FollowerAnimIns->Montage_Play(Montage);

		if (bShouldSync)
		{
			auto* OtherAnimIns{ MeshComp->GetAnimInstance() };

			auto* MontageLeader{ Cast<UAnimMontage>(Animation) };

			FollowerAnimIns->MontageSync_Follow(Montage, OtherAnimIns, MontageLeader);
		}
	}
}

void UAnimNotifyState_PlayPairMontage::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!bShouldSync && bForceEndPairMontage)
	{
		auto* LocomotionCharacter{ MeshComp->GetOwner<ALocomotionCharacter>() };
		auto* Mesh{ LocomotionCharacter ? ICharacterMeshAccessorInterface::Execute_GetMeshByTag(LocomotionCharacter, MeshTag) : nullptr };

		if (auto* AnimIns{ Mesh ? Mesh->GetAnimInstance() : nullptr })
		{
			AnimIns->Montage_Stop(BlendOutTime, Montage);
		}
	}
}

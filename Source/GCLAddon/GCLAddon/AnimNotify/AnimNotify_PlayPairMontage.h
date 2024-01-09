// Copyright (C) 2024 owoDra

#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"

#include "GameplayTagContainer.h"

#include "AnimNotify_PlayPairMontage.generated.h"

class UAnimMontage;


/**
 * AnimNotify class that plays the specified animation for other Mesh of the character during animation playback
 */
UCLASS(BlueprintType, meta = (DisplayName = "AN Play Pair Montage"))
class UAnimNotify_PlayPairMontage : public UAnimNotify
{
	GENERATED_BODY()
public:
	UAnimNotify_PlayPairMontage(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlayMontage", meta = (Categories = "MeshType"))
	FGameplayTag MeshTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlayMontage")
	TObjectPtr<UAnimMontage> Montage;

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp, 
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

};
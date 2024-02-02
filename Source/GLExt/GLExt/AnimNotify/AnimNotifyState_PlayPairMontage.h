// Copyright (C) 2024 owoDra

#pragma once

#include "Animation/AnimNotifies/AnimNotifyState.h"

#include "GameplayTagContainer.h"

#include "AnimNotifyState_PlayPairMontage.generated.h"

class UAnimMontage;


/**
 * AnimNotifyState class that plays the specified animation for other Mesh of the character during animation playback
 */
UCLASS(BlueprintType, meta = (DisplayName = "ANS Play Pair Montage"))
class UAnimNotifyState_PlayPairMontage : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UAnimNotifyState_PlayPairMontage(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlayMontage", meta = (Categories = "MeshType"))
	FGameplayTag MeshTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlayMontage")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(AdvancedDisplay, BlueprintReadWrite, EditAnywhere, Category = "PlayMontage")
	bool bShouldSync{ true };

	UPROPERTY(AdvancedDisplay, BlueprintReadWrite, EditAnywhere, Category = "PlayMontage", meta = (EditCondition = "!bShouldSync"))
	bool bForceEndPairMontage{ true };

	UPROPERTY(AdvancedDisplay, BlueprintReadWrite, EditAnywhere, Category = "PlayMontage", meta = (EditCondition = "!bShouldSync && bForceEndPairMontage"))
	float BlendOutTime{ 0.2f };

public:
	virtual FString GetNotifyName_Implementation() const override;

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
// Copyright (C) 2024 owoDra

#pragma once

#include "AnimNodes/AnimNode_BlendListBase.h"

#include "GameplayTagContainer.h"

#include "AnimNode_GameplayTagsBlend.generated.h"


/**
 * AnimNode class to blend animations based on GameplayTag
 */
USTRUCT()
struct GCLADDON_API FAnimNode_GameplayTagsBlend : public FAnimNode_BlendListBase
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Settings", Meta = (FoldProperty, PinShownByDefault))
	FGameplayTag ActiveTag;

	UPROPERTY(EditAnywhere, Category = "Settings", Meta = (FoldProperty))
	TArray<FGameplayTag> Tags;
#endif

protected:
	virtual int32 GetActiveChildIndex() override;

public:
	const FGameplayTag& GetActiveTag() const;

	const TArray<FGameplayTag>& GetTags() const;

#if WITH_EDITOR
	void RefreshPoses();
#endif

};

// Copyright (C) 2024 owoDra

#pragma once

#include "AnimGraphNode_Base.h"

#include "Node/AnimNode_CurvesBlend.h"

#include "AnimGraphNode_CurvesBlend.generated.h"

/**
 * Class that defines the use of CurvesBlend's AnimNode in the editor
 */
UCLASS()
class GCLADDONNODE_API UAnimGraphNode_CurvesBlend : public UAnimGraphNode_Base
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAnimNode_CurvesBlend Node;

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual FText GetTooltipText() const override;

	virtual FString GetNodeCategory() const override;

};

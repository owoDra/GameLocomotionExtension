// Copyright (C) 2023 owoDra

#include "AnimGraphNode_CurvesBlend.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimGraphNode_CurvesBlend)


#define LOCTEXT_NAMESPACE "CurvesBlendAnimationGraphNode"

FText UAnimGraphNode_CurvesBlend::GetNodeTitle(const ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Title", "Blend Curves");
}

FText UAnimGraphNode_CurvesBlend::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Blend Curves");
}

FString UAnimGraphNode_CurvesBlend::GetNodeCategory() const
{
	return FString{ TEXTVIEW("Blend") };
}

#undef LOCTEXT_NAMESPACE

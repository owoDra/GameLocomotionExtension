// Copyright (C) 2024 owoDra

#include "AnimNode_CurvesBlend.h"

#include "Animation/AnimTrace.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimNode_CurvesBlend)


void FAnimNode_CurvesBlend::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Initialize_AnyThread)

	Super::Initialize_AnyThread(Context);

	SourcePose.Initialize(Context);
	CurvesPose.Initialize(Context);
}

void FAnimNode_CurvesBlend::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(CacheBones_AnyThread)

	Super::CacheBones_AnyThread(Context);

	SourcePose.CacheBones(Context);
	CurvesPose.CacheBones(Context);
}

void FAnimNode_CurvesBlend::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Update_AnyThread)

	Super::Update_AnyThread(Context);

	GetEvaluateGraphExposedInputs().Execute(Context);

	SourcePose.Update(Context);

	const auto CurrentBlendAmount{GetBlendAmount()};
	if (FAnimWeight::IsRelevant(CurrentBlendAmount))
	{
		CurvesPose.Update(Context);
	}

	TRACE_ANIM_NODE_VALUE(Context, TEXT("Blend Amount"), CurrentBlendAmount);
	TRACE_ANIM_NODE_VALUE(Context, TEXT("Blend Mode"), *StaticEnum<ECurvesBlendMode>()->GetNameStringByValue(static_cast<int64>(GetBlendMode())));
}

void FAnimNode_CurvesBlend::Evaluate_AnyThread(FPoseContext& Output)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Evaluate_AnyThread)

	Super::Evaluate_AnyThread(Output);

	SourcePose.Evaluate(Output);

	const auto CurrentBlendAmount{GetBlendAmount()};
	if (!FAnimWeight::IsRelevant(CurrentBlendAmount))
	{
		return;
	}

	auto CurvesPoseContext{Output};
	CurvesPose.Evaluate(CurvesPoseContext);

	switch (GetBlendMode())
	{
		case ECurvesBlendMode::BlendByAmount:
			Output.Curve.Accumulate(CurvesPoseContext.Curve, CurrentBlendAmount);
			break;

		case ECurvesBlendMode::Combine:
			Output.Curve.Combine(CurvesPoseContext.Curve);
			break;

		case ECurvesBlendMode::CombinePreserved:
			Output.Curve.CombinePreserved(CurvesPoseContext.Curve);
			break;

		case ECurvesBlendMode::UseMaxValue:
			Output.Curve.UseMaxValue(CurvesPoseContext.Curve);
			break;

		case ECurvesBlendMode::UseMinValue:
			Output.Curve.UseMinValue(CurvesPoseContext.Curve);
			break;

		case ECurvesBlendMode::Override:
			Output.Curve.Override(CurvesPoseContext.Curve);
			break;
	}
}

void FAnimNode_CurvesBlend::GatherDebugData(FNodeDebugData& DebugData)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(GatherDebugData)

	TStringBuilder<256> DebugItemBuilder;

	DebugItemBuilder << DebugData.GetNodeName(this) << TEXTVIEW(": Blend Amount: ");
	DebugItemBuilder.Appendf(TEXT("%.2f"), GetBlendAmount());

	DebugData.AddDebugItem(FString{DebugItemBuilder});
	SourcePose.GatherDebugData(DebugData.BranchFlow(1.0f));
	CurvesPose.GatherDebugData(DebugData.BranchFlow(GetBlendAmount()));
}

float FAnimNode_CurvesBlend::GetBlendAmount() const
{
	return GET_ANIM_NODE_DATA(float, BlendAmount);
}

ECurvesBlendMode FAnimNode_CurvesBlend::GetBlendMode() const
{
	return GET_ANIM_NODE_DATA(ECurvesBlendMode, BlendMode);
}

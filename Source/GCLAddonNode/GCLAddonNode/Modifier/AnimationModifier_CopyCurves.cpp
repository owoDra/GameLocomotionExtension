// Copyright (C) 2023 owoDra

#include "AnimationModifier_CopyCurves.h"

#include "AnimationBlueprintLibrary.h"
#include "Animation/AnimSequence.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimationModifier_CopyCurves)


void UAnimationModifier_CopyCurves::OnApply_Implementation(UAnimSequence* Sequence)
{
	Super::OnApply_Implementation(Sequence);

	auto* SourceSequenceObject{SourceSequence.LoadSynchronous()};
	if (!ensure(IsValid(SourceSequenceObject)))
	{
		return;
	}

	if (bCopyAllCurves)
	{
		for (const auto& Curve : SourceSequenceObject->GetCurveData().FloatCurves)
		{
			CopyCurve(SourceSequenceObject, Sequence, Curve.GetName());
		}
	}
	else
	{
		for (const auto& CurveName : CurveNames)
		{
			if (UAnimationBlueprintLibrary::DoesCurveExist(SourceSequenceObject, CurveName, ERawCurveTrackTypes::RCT_Float))
			{
				CopyCurve(SourceSequenceObject, Sequence, CurveName);
			}
		}
	}
}

void UAnimationModifier_CopyCurves::CopyCurve(UAnimSequence* SourceSequence, UAnimSequence* TargetSequence, const FName& CurveName)
{
	static TArray<float> CurveTimes;
	check(CurveTimes.IsEmpty())

	static TArray<float> CurveValues;
	check(CurveValues.IsEmpty())

	ON_SCOPE_EXIT
	{
		CurveTimes.Reset();
		CurveValues.Reset();
	};

	if (UAnimationBlueprintLibrary::DoesCurveExist(TargetSequence, CurveName, ERawCurveTrackTypes::RCT_Float))
	{
		UAnimationBlueprintLibrary::RemoveCurve(TargetSequence, CurveName);
	}

	UAnimationBlueprintLibrary::AddCurve(TargetSequence, CurveName);

	UAnimationBlueprintLibrary::GetFloatKeys(SourceSequence, CurveName, CurveTimes, CurveValues);
	UAnimationBlueprintLibrary::AddFloatCurveKeys(TargetSequence, CurveName, CurveTimes, CurveValues);
}

#include "AnimationModifier_CalculateRotationYawSpeed.h"

#include "LocomotionGeneralNameStatics.h"

#include "Animation/AnimSequence.h"
#include "AnimationBlueprintLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimationModifier_CalculateRotationYawSpeed)


void UAnimationModifier_CalculateRotationYawSpeed::OnApply_Implementation(UAnimSequence* Sequence)
{
	Super::OnApply_Implementation(Sequence);

	if (UAnimationBlueprintLibrary::DoesCurveExist(Sequence, ULocomotionGeneralNameStatics::RotationYawSpeedCurveName(), ERawCurveTrackTypes::RCT_Float))
	{
		UAnimationBlueprintLibrary::RemoveCurve(Sequence, ULocomotionGeneralNameStatics::RotationYawSpeedCurveName());
	}

	UAnimationBlueprintLibrary::AddCurve(Sequence, ULocomotionGeneralNameStatics::RotationYawSpeedCurveName());

	const auto* DataModel{Sequence->GetDataModel()};
	const auto FrameRate{Sequence->GetSamplingFrameRate().AsDecimal()};

	UAnimationBlueprintLibrary::AddFloatCurveKey(Sequence, ULocomotionGeneralNameStatics::RotationYawSpeedCurveName(), 0.0f, 0.0f);

	for (auto i{1}; i < Sequence->GetNumberOfSampledKeys(); i++)
	{
		auto CurrentPoseTransform{
			DataModel->GetBoneTrackTransform(ULocomotionGeneralNameStatics::RootBoneName(), i + (Sequence->RateScale >= 0.0f ? -1 : 0))
		};

		auto NextPoseTransform{
			DataModel->GetBoneTrackTransform(ULocomotionGeneralNameStatics::RootBoneName(), i + (Sequence->RateScale >= 0.0f ? 0 : -1))
		};

		UAnimationBlueprintLibrary::AddFloatCurveKey(Sequence, ULocomotionGeneralNameStatics::RotationYawSpeedCurveName(), Sequence->GetTimeAtFrame(i),
		                                             UE_REAL_TO_FLOAT(NextPoseTransform.Rotator().Yaw - CurrentPoseTransform.Rotator().Yaw) *
		                                             FMath::Abs(Sequence->RateScale) * FrameRate);
	}
}

#include "AnimGraphNode_GameplayTagsBlend.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimGraphNode_GameplayTagsBlend)


#define LOCTEXT_NAMESPACE "GameplayTagsBlendAnimationGraphNode"

UAnimGraphNode_GameplayTagsBlend::UAnimGraphNode_GameplayTagsBlend()
{
	Node.AddPose();
}

void UAnimGraphNode_GameplayTagsBlend::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(FAnimNode_GameplayTagsBlend, Tags))
	{
		ReconstructNode();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

FText UAnimGraphNode_GameplayTagsBlend::GetNodeTitle(const ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Title", "Blend Poses by Gameplay Tag");
}

FText UAnimGraphNode_GameplayTagsBlend::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Blend Poses by Gameplay Tag");
}

void UAnimGraphNode_GameplayTagsBlend::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& PreviousPins)
{
	Node.RefreshPoses();

	Super::ReallocatePinsDuringReconstruction(PreviousPins);
}

FString UAnimGraphNode_GameplayTagsBlend::GetNodeCategory() const
{
	return FString{TEXTVIEW("Blend")};
}

void UAnimGraphNode_GameplayTagsBlend::CustomizePinData(UEdGraphPin* Pin, const FName SourcePropertyName, const int32 PinIndex) const
{
	Super::CustomizePinData(Pin, SourcePropertyName, PinIndex);

	bool bBlendPosePin;
	bool bBlendTimePin;
	GetBlendPinProperties(Pin, bBlendPosePin, bBlendTimePin);

	if (!bBlendPosePin && !bBlendTimePin)
	{
		return;
	}

	Pin->PinFriendlyName = PinIndex <= 0
		                       ? LOCTEXT("Default", "Default")
		                       : PinIndex > Node.Tags.Num()
		                       ? LOCTEXT("Invalid", "Invalid")
		                       : FText::AsCultureInvariant(Node.Tags[PinIndex - 1].GetTagName().ToString());

	if (bBlendPosePin)
	{
		static const FTextFormat BlendPosePinFormat{LOCTEXT("Pose", "{PinName} Pose")};

		Pin->PinFriendlyName = FText::Format(BlendPosePinFormat, {{FString{TEXTVIEW("PinName")}, Pin->PinFriendlyName}});
	}
	else if (bBlendTimePin)
	{
		static const FTextFormat BlendTimePinFormat{LOCTEXT("BlendTime", "{PinName} Blend Time")};

		Pin->PinFriendlyName = FText::Format(BlendTimePinFormat, {{FString{TEXTVIEW("PinName")}, Pin->PinFriendlyName}});
	}
}

void UAnimGraphNode_GameplayTagsBlend::GetBlendPinProperties(const UEdGraphPin* Pin, bool& bBlendPosePin, bool& bBlendTimePin)
{
	const auto PinFullName{Pin->PinName.ToString()};
	const auto SeparatorIndex{PinFullName.Find(TEXT("_"), ESearchCase::CaseSensitive)};

	if (SeparatorIndex <= 0)
	{
		bBlendPosePin = false;
		bBlendTimePin = false;
		return;
	}

	const auto PinName{PinFullName.Left(SeparatorIndex)};
	bBlendPosePin = PinName == TEXTVIEW("BlendPose");
	bBlendTimePin = PinName == TEXTVIEW("BlendTime");
}

#undef LOCTEXT_NAMESPACE

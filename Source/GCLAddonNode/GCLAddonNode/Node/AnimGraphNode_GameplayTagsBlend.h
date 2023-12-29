#pragma once

#include "AnimGraphNode_BlendListBase.h"

#include "Node/AnimNode_GameplayTagsBlend.h"

#include "AnimGraphNode_GameplayTagsBlend.generated.h"


/**
 * Class that defines to use AnimNode of GameplayTagsBlend in the editor
 */
UCLASS()
class GCLADDONNODE_API UAnimGraphNode_GameplayTagsBlend : public UAnimGraphNode_BlendListBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAnimNode_GameplayTagsBlend Node;

public:
	UAnimGraphNode_GameplayTagsBlend();

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	virtual FText GetTooltipText() const override;

	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& PreviousPins) override;

	virtual FString GetNodeCategory() const override;

	virtual void CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 PinIndex) const override;

protected:
	static void GetBlendPinProperties(const UEdGraphPin* Pin, bool& bBlendPosePin, bool& bBlendTimePin);

};

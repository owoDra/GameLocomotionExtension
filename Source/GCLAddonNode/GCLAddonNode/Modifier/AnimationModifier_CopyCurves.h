// Copyright (C) 2023 owoDra

#pragma once

#include "AnimationModifier.h"

#include "AnimationModifier_CopyCurves.generated.h"

/**
 * AnimModifier class to copy AnimCurves of an AnimSeaquence to another AnimSeaquence
 */
UCLASS(DisplayName = "Copy Curves Animation Modifier")
class GCLADDONNODE_API UAnimationModifier_CopyCurves : public UAnimationModifier
{
public:
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	TSoftObjectPtr<UAnimSequence> SourceSequence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bCopyAllCurves{true};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (EditCondition = "!bCopyAllCurves"))
	TArray<FName> CurveNames;

public:
	virtual void OnApply_Implementation(UAnimSequence* Sequence) override;

private:
	static void CopyCurve(UAnimSequence* SourceSequence, UAnimSequence* TargetSequence, const FName& CurveName);

};

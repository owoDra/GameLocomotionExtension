// Copyright (C) 2024 owoDra

#pragma once

#include "Animation/Skeleton.h"

#include "SkeletonFunctionLibrary.generated.h"


USTRUCT(BlueprintType)
struct GLEXTNODE_API FBlendProfileEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendScale{ 1.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bIncludeDescendants : 1 { false };

};


UCLASS()
class GLEXTNODE_API USkeletonFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Skeleton")
	static void AddAnimationCurves(USkeleton* Skeleton, const TArray<FName>& CurveNames);

	UFUNCTION(BlueprintCallable, Category = "Skeleton")
	static void AddOrReplaceSlot(USkeleton* Skeleton, FName SlotName, FName GroupName);

	UFUNCTION(BlueprintCallable, Category = "Skeleton", Meta = (AutoCreateRefTerm = "SourceBoneName, TargetBoneName"))
	static void AddOrReplaceVirtualBone(USkeleton* Skeleton, const FName& SourceBoneName, const FName& TargetBoneName, FName VirtualBoneName);

	UFUNCTION(BlueprintCallable, Category = "Skeleton", Meta = (AutoCreateRefTerm = "BoneName, RelativeLocation, RelativeRotation"))
	static void AddOrReplaceSocket(USkeleton* Skeleton, FName SocketName, const FName& BoneName, const FVector& RelativeLocation, const FRotator& RelativeRotation);

	UFUNCTION(BlueprintCallable, Category = "Skeleton")
	static void AddOrReplaceWeightBlendProfile(USkeleton* Skeleton, FName BlendProfileName, const TArray<FBlendProfileEntry>& Entries);

	UFUNCTION(BlueprintCallable, Category = "Skeleton", Meta = (AutoCreateRefTerm = "BoneName"))
	static void SetBoneRetargetingMode(USkeleton* Skeleton, const FName& BoneName, EBoneTranslationRetargetingMode::Type RetargetingMode, bool bIncludeDescendants);

};

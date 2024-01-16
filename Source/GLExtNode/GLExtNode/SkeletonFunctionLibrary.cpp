// Copyright (C) 2024 owoDra

#include "SkeletonFunctionLibrary.h"

#include "Animation/BlendProfile.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SkeletonFunctionLibrary)


#define LOCTEXT_NAMESPACE "SkeletonFunctionLibrary"

namespace SkeletonFunctionLibrary
{
	const FName NAME_MessageLog{ TEXT("LogGLE") };

	void LogMissingBoneWarning(const USkeleton* Skeleton, const FName& BoneName, const FString& FunctionName)
	{
		auto MessageLog{ FMessageLog(SkeletonFunctionLibrary::NAME_MessageLog) };

		MessageLog.Warning(FText::Format(
			          LOCTEXT("MissingBoneWarning", "{FunctionName}: Bone {BoneName} does not exist on the {SkeletonName} skeleton!"),
			          {
				          {FString{TEXTVIEW("FunctionName")}, FText::AsCultureInvariant(FunctionName)},
				          {FString{TEXTVIEW("BoneName")}, FText::AsCultureInvariant(BoneName.ToString())},
				          {FString{TEXTVIEW("SkeletonName")}, FText::AsCultureInvariant(Skeleton->GetName())}
			          }))
		          ->AddToken(FUObjectToken::Create(Skeleton));

		MessageLog.Open(EMessageSeverity::Warning);
	}
}

void USkeletonFunctionLibrary::AddAnimationCurves(USkeleton* Skeleton, const TArray<FName>& CurveNames)
{
	if (!ensure(Skeleton))
	{
		return;
	}

	auto MessageLog{ FMessageLog(SkeletonFunctionLibrary::NAME_MessageLog) };

	for (auto CurveName : CurveNames)
	{
		CurveName = *CurveName.ToString().TrimStartAndEnd();
		if (CurveName.IsNone())
		{
			MessageLog.Warning(FText::Format(
				LOCTEXT("EmptyCurveNameWarning", "{FunctionName}: Curve must have a valid name!"),
				{
					{FString{TEXTVIEW("FunctionName")}, FText::AsCultureInvariant(__FUNCTION__)}
				}));

			MessageLog.Open(EMessageSeverity::Warning);
			continue;
		}

		Skeleton->AddCurveMetaData(CurveName);
	}
}

void USkeletonFunctionLibrary::AddOrReplaceSlot(USkeleton* Skeleton, FName SlotName, FName GroupName)
{
	if (!ensure(Skeleton))
	{
		return;
	}

	SlotName = *SlotName.ToString().TrimStartAndEnd();
	if (SlotName.IsNone())
	{
		auto MessageLog{ FMessageLog(SkeletonFunctionLibrary::NAME_MessageLog) };


		MessageLog.Warning(FText::Format(
			LOCTEXT("EmptySlotNameWarning", "{FunctionName}: Slot must have a valid name!"),
			{
				{FString{TEXTVIEW("FunctionName")}, FText::AsCultureInvariant(__FUNCTION__)}
			}));

		MessageLog.Open(EMessageSeverity::Warning);
		return;
	}

	GroupName = *GroupName.ToString().TrimStartAndEnd();
	if (GroupName.IsNone())
	{
		auto MessageLog{ FMessageLog(SkeletonFunctionLibrary::NAME_MessageLog) };

		MessageLog.Warning(FText::Format(
			LOCTEXT("EmptySlotGroupNameWarning", "{FunctionName}: Slot group must have a valid name!"),
			{
				{FString{TEXTVIEW("FunctionName")}, FText::AsCultureInvariant(__FUNCTION__)}
			}));

		MessageLog.Open(EMessageSeverity::Warning);
		return;
	}

	if (!Skeleton->ContainsSlotName(SlotName) || Skeleton->GetSlotGroupName(SlotName) != GroupName)
	{
		Skeleton->Modify();
		Skeleton->SetSlotGroupName(SlotName, GroupName);
	}
}

void USkeletonFunctionLibrary::AddOrReplaceVirtualBone(USkeleton* Skeleton, const FName& SourceBoneName, const FName& TargetBoneName, FName VirtualBoneName)
{
	if (!ensure(Skeleton))
	{
		return;
	}

	const auto VirtualBoneString{VirtualBoneName.ToString().TrimStartAndEnd()};
	VirtualBoneName = *VirtualBoneString;

	if (VirtualBoneName.IsNone())
	{
		auto MessageLog{ FMessageLog(SkeletonFunctionLibrary::NAME_MessageLog) };

		MessageLog.Warning(FText::Format(
			LOCTEXT("EmptyVirtualBoneNameWarning", "{FunctionName}: Virtual bone must have a valid name!"),
			{
				{FString{TEXTVIEW("FunctionName")}, FText::AsCultureInvariant(__FUNCTION__)}
			}));

		MessageLog.Open(EMessageSeverity::Warning);
		return;
	}

	if (!VirtualBoneString.StartsWith(TEXTVIEW("VB "), ESearchCase::CaseSensitive))
	{
		auto MessageLog{ FMessageLog(SkeletonFunctionLibrary::NAME_MessageLog) };

		MessageLog.Warning(FText::Format(
			LOCTEXT("NoVirtualBonePrefixWarning", "{FunctionName}: Virtual bone {VirtualBoneName} must contain the \"VB \" prefix!"),
			{
				{FString{TEXTVIEW("FunctionName")}, FText::AsCultureInvariant(__FUNCTION__)},
				{FString{TEXTVIEW("VirtualBoneName")}, FText::AsCultureInvariant(VirtualBoneString)}
			}));

		MessageLog.Open(EMessageSeverity::Warning);
		return;
	}

	const auto* ExistingVirtualBone{
		Skeleton->GetVirtualBones().FindByPredicate([&VirtualBoneName](const FVirtualBone& VirtualBone)
		{
			return VirtualBone.VirtualBoneName == VirtualBoneName;
		})
	};

	if (ExistingVirtualBone != nullptr &&
	    ExistingVirtualBone->SourceBoneName == SourceBoneName &&
	    ExistingVirtualBone->TargetBoneName == TargetBoneName)
	{
		return;
	}

	if (Skeleton->GetReferenceSkeleton().FindBoneIndex(SourceBoneName) < 0)
	{
		SkeletonFunctionLibrary::LogMissingBoneWarning(Skeleton, SourceBoneName, __FUNCTION__);
		return;
	}

	if (Skeleton->GetReferenceSkeleton().FindBoneIndex(TargetBoneName) < 0)
	{
		SkeletonFunctionLibrary::LogMissingBoneWarning(Skeleton, TargetBoneName, __FUNCTION__);
		return;
	}

	if (ExistingVirtualBone != nullptr)
	{
		ExistingVirtualBone = nullptr;

		Skeleton->RemoveVirtualBones({VirtualBoneName});
	}

	FName TempVirtualBoneName;

	Skeleton->AddNewVirtualBone(SourceBoneName, TargetBoneName, TempVirtualBoneName);
	Skeleton->RenameVirtualBone(TempVirtualBoneName, VirtualBoneName);
}

void USkeletonFunctionLibrary::AddOrReplaceSocket(USkeleton* Skeleton, FName SocketName, const FName& BoneName, const FVector& RelativeLocation, const FRotator& RelativeRotation)
{
	if (!ensure(Skeleton))
	{
		return;
	}

	SocketName = *SocketName.ToString().TrimStartAndEnd();
	if (SocketName.IsNone())
	{
		auto MessageLog{ FMessageLog(SkeletonFunctionLibrary::NAME_MessageLog) };

		MessageLog.Warning(FText::Format(
			LOCTEXT("EmptySocketNameWarning", "{FunctionName}: Socket must have a valid name!"),
			{
				{FString{TEXTVIEW("FunctionName")}, FText::AsCultureInvariant(__FUNCTION__)}
			}));

		MessageLog.Open(EMessageSeverity::Warning);
		return;
	}

	if (Skeleton->GetReferenceSkeleton().FindBoneIndex(BoneName) < 0)
	{
		SkeletonFunctionLibrary::LogMissingBoneWarning(Skeleton, BoneName, __FUNCTION__);
		return;
	}

	auto* ExistingSocket{Skeleton->FindSocket(SocketName)};
	if (IsValid(ExistingSocket))
	{
		if (ExistingSocket->BoneName != BoneName ||
		    ExistingSocket->RelativeLocation != RelativeLocation ||
		    ExistingSocket->RelativeRotation != RelativeRotation ||
		    ExistingSocket->RelativeScale != FVector::OneVector ||
		    !ExistingSocket->bForceAlwaysAnimated)
		{
			Skeleton->Modify();
			ExistingSocket->Modify();

			ExistingSocket->BoneName = BoneName;
			ExistingSocket->RelativeLocation = RelativeLocation;
			ExistingSocket->RelativeRotation = RelativeRotation;
			ExistingSocket->RelativeScale = FVector::OneVector;
			ExistingSocket->bForceAlwaysAnimated = true;
		}

		return;
	}

	Skeleton->Modify();

	auto* Socket{NewObject<USkeletalMeshSocket>(Skeleton)};
	Socket->SocketName = SocketName;
	Socket->BoneName = BoneName;
	Socket->RelativeLocation = RelativeLocation;
	Socket->RelativeRotation = RelativeRotation;

	Skeleton->Sockets.Emplace(Socket);
}

void USkeletonFunctionLibrary::AddOrReplaceWeightBlendProfile(USkeleton* Skeleton, FName BlendProfileName, const TArray<FBlendProfileEntry>& Entries)
{
	if (!ensure(Skeleton))
	{
		return;
	}

	BlendProfileName = *BlendProfileName.ToString().TrimStartAndEnd();
	if (BlendProfileName.IsNone())
	{
		auto MessageLog{ FMessageLog(SkeletonFunctionLibrary::NAME_MessageLog) };

		MessageLog.Warning(FText::Format(
			LOCTEXT("EmptyBlendProfileNameNameWarning", "{FunctionName}: Blend profile must have a valid name!"),
			{
				{FString{TEXTVIEW("FunctionName")}, FText::AsCultureInvariant(__FUNCTION__)}
			}));

		MessageLog.Open(EMessageSeverity::Warning);
		return;
	}

	auto* BlendProfile{Skeleton->GetBlendProfile(BlendProfileName)};
	if (IsValid(BlendProfile))
	{
		Skeleton->Modify();
		BlendProfile->Modify();
		BlendProfile->ProfileEntries.Reset();
	}
	else
	{
		BlendProfile = Skeleton->CreateNewBlendProfile(BlendProfileName);
	}

	BlendProfile->Mode = EBlendProfileMode::WeightFactor;

	for (const auto& Entry : Entries)
	{
		const auto BoneIndex{Skeleton->GetReferenceSkeleton().FindBoneIndex(Entry.BoneName)};
		if (BoneIndex < 0)
		{
			SkeletonFunctionLibrary::LogMissingBoneWarning(Skeleton, Entry.BoneName, __FUNCTION__);
			continue;
		}

		BlendProfile->SetBoneBlendScale(BoneIndex, Entry.BlendScale, Entry.bIncludeDescendants, true);
	}
}

void USkeletonFunctionLibrary::SetBoneRetargetingMode(USkeleton* Skeleton, const FName& BoneName, const EBoneTranslationRetargetingMode::Type RetargetingMode, const bool bIncludeDescendants)
{
	if (!ensure(Skeleton))
	{
		return;
	}

	const auto BoneIndex{Skeleton->GetReferenceSkeleton().FindBoneIndex(BoneName)};
	if (BoneIndex < 0)
	{
		SkeletonFunctionLibrary::LogMissingBoneWarning(Skeleton, BoneName, __FUNCTION__);
		return;
	}

	Skeleton->Modify();
	Skeleton->SetBoneTranslationRetargetingMode(BoneIndex, RetargetingMode, bIncludeDescendants);
}

#undef LOCTEXT_NAMESPACE

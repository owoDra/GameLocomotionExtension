// Copyright (C) 2023 owoDra

#include "CharacterModifier_ApplyLocomotionData.h"

#include "GCLAddonLogs.h"

#include "LocomotionComponent.h"
#include "LocomotionData.h"

#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterModifier_ApplyLocomotionData)


UCharacterModifier_ApplyLocomotionData::UCharacterModifier_ApplyLocomotionData()
{
	bOnlyApplyOnLocal = false;
	bApplyOnClient = true;
	bApplyOnServer = true;
}


bool UCharacterModifier_ApplyLocomotionData::OnApply(APawn* Pawn) const
{
	const auto bCanApply{ Super::OnApply(Pawn) };

	if (bCanApply)
	{
		if (auto* Character{ Cast<ACharacter>(Pawn) })
		{
			if (auto* LC{ ULocomotionComponent::FindLocomotionComponent(Character) })
			{
				auto* LoadedLocomotionData
				{
					LocomotionData.IsNull() ? nullptr :
					LocomotionData.IsValid() ? LocomotionData.Get() : LocomotionData.LoadSynchronous()
				};

				UE_LOG(LogGCLA, Log, TEXT("++LocomotionData (Name: %s)"), *GetNameSafe(LoadedLocomotionData));

				LC->SetLocomotionData(LoadedLocomotionData);
			}
		}
	}

	return bCanApply;
}

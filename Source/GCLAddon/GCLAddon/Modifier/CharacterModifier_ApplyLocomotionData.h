// Copyright (C) 2023 owoDra

#pragma once

#include "CharacterModifier.h"

#include "CharacterModifier_ApplyLocomotionData.generated.h"

class ULocomotionData;


/**
 * Modifier class to apply locomotion data to Character
 */
UCLASS(meta = (DisplayName = "CM Apply Locomotion Data"))
class UCharacterModifier_ApplyLocomotionData final : public UCharacterModifier
{
	GENERATED_BODY()
public:
	UCharacterModifier_ApplyLocomotionData();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "ApplyLocomotionData")
	TSoftObjectPtr<ULocomotionData> LocomotionData{ nullptr };

protected:
	virtual bool OnApply(APawn* Pawn) const override;

};

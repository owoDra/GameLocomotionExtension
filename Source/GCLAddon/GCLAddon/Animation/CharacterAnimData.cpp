// Copyright (C) 2023 owoDra

#include "CharacterAnimData.h"

#include "Engine/EngineTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterAnimData)


/////////////////////////////////////////
// FAnimInAirConfigs

FAnimInAirConfigs::FAnimInAirConfigs()
{
	GroundPredictionSweepResponses = ECR_Ignore;
}

#if WITH_EDITOR
void FAnimInAirConfigs::PostEditChangeProperty(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() != GET_MEMBER_NAME_CHECKED(FAnimInAirConfigs, GroundPredictionSweepObjectTypes))
	{
		return;
	}

	GroundPredictionSweepResponses.SetAllChannels(ECR_Ignore);

	for (const auto ObjectType : GroundPredictionSweepObjectTypes)
	{
		GroundPredictionSweepResponses.SetResponse(UEngineTypes::ConvertToCollisionChannel(ObjectType), ECR_Block);
	}
}
#endif


/////////////////////////////////////////
// UCharacterAnimConfigs

UCharacterAnimData::UCharacterAnimData()
{
	InAir.GroundPredictionSweepObjectTypes =
	{
		UEngineTypes::ConvertToObjectType(ECC_WorldStatic),
		UEngineTypes::ConvertToObjectType(ECC_WorldDynamic),
		UEngineTypes::ConvertToObjectType(ECC_Destructible)
	};

	InAir.GroundPredictionSweepResponses.SetResponse(ECC_WorldStatic, ECR_Block);
	InAir.GroundPredictionSweepResponses.SetResponse(ECC_WorldDynamic, ECR_Block);
	InAir.GroundPredictionSweepResponses.SetResponse(ECC_Destructible, ECR_Block);
}

void UCharacterAnimData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	InAir.PostEditChangeProperty(PropertyChangedEvent);

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

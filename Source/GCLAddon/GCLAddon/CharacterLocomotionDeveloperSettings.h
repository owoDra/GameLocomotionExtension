// Copyright (C) 2023 owoDra

#pragma once

#include "Engine/DeveloperSettings.h"

#include "CharacterLocomotionDeveloperSettings.generated.h"


/**
 * Settings for a Character Locomotion
 */
UCLASS(Config = "Game", Defaultconfig, meta = (DisplayName = "Game Character: Locomotion Addon"))
class UCharacterLocomotionDeveloperSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
public:
	UCharacterLocomotionDeveloperSettings();

	///////////////////////////////////////////////
	// Collision Profile
public:
	//
	// Collision profile name of the character's capsule collision
	//
	UPROPERTY(Config, EditAnywhere, Category = "Collision Profile")
	FName CapsuleCollisionProfileName{ TEXT("PawnCapsule") };

	//
	// Collision profile name of the character's mesh collision
	//
	UPROPERTY(Config, EditAnywhere, Category = "Collision Profile")
	FName MeshCollisionProfileName{ TEXT("PawnMesh") };

};


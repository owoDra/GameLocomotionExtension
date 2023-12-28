// Copyright (C) 2023 owoDra

#pragma once

#include "Engine/DataAsset.h"

#include "CharacterAnimData.generated.h"

class UCurveFloat;
class UAnimSequenceBase;


/**
 *  Data on basic animation settings related to the character
 */
USTRUCT(BlueprintType)
struct GCLADDON_API FCharacterAnimGeneralConfigs
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseHandIkBones{ true };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseFootIkBones{ true };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float MovingSmoothSpeedThreshold{ 150.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0))
	float LeanInterpolationSpeed{ 4.0f };

};


/**
 * Data for animation settings such as IK of feet related to the character
 */
USTRUCT(BlueprintType)
struct GCLADDON_API FCharacterAnimFeetConfigs
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDisableFootLock{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "cm"))
	float FootHeight{ 13.5f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ETraceTypeQuery> IkTraceChannel{ TraceTypeQuery1 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "cm"))
	float IkTraceDistanceUpward{ 50.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "cm"))
	float IkTraceDistanceDownward{ 45.0f };

};


/**
 * Data for setting up animations based on viewpoints related to the character
 */
USTRUCT(BlueprintType)
struct GCLADDON_API FCharacterAnimViewConfigs
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0))
	float LookTowardsCameraRotationInterpolationSpeed{ 8.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0))
	float LookTowardsInputYawAngleInterpolationSpeed{ 8.0f };

};


/**
 * Data on the animation settings when on the ground regarding the character
 */
USTRUCT(BlueprintType)
struct GCLADDON_API FCharacterAnimGroundedConfigs
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float AnimatedWalkSpeed{ 150.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float AnimatedRunSpeed{ 350.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float AnimatedSprintSpeed{ 600.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float AnimatedCrouchSpeed{ 150.0f };

	// 
	// Blend Amount Curves for Travel Speed and Stride
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCurveFloat> StrideBlendAmountWalkCurve{ nullptr };

	// 
	// Blend Amount Curves for Travel Speed and Stride
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCurveFloat> StrideBlendAmountRunCurve{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCurveFloat> RotationYawOffsetForwardCurve{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCurveFloat> RotationYawOffsetBackwardCurve{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCurveFloat> RotationYawOffsetLeftCurve{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCurveFloat> RotationYawOffsetRightCurve{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0))
	float VelocityBlendInterpolationSpeed{ 12.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float PivotActivationSpeedThreshold{ 200.0f };

};


/**
 * FAnimInAirConfigs
 *
 *  地上にいる時のアニメーションの設定を定義する
 */
USTRUCT(BlueprintType)
struct GCLADDON_API FAnimInAirConfigs
{
	GENERATED_BODY()
public:
	FAnimInAirConfigs();

#if WITH_EDITOR
	void PostEditChangeProperty(const FPropertyChangedEvent& PropertyChangedEvent);
#endif

public:
	//
	// 落下速度と Lean 量のカーブ
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCurveFloat> LeanAmountCurve{ nullptr };

	//
	// 地面予測スイープヒット時間から地面予測量のカーブ
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCurveFloat> GroundPredictionAmountCurve{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TEnumAsByte<EObjectTypeQuery>> GroundPredictionSweepObjectTypes;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FCollisionResponseContainer GroundPredictionSweepResponses;
};


/**
 * FAnimRotateInPlaceConfigs
 *
 *  その場回転のアニメーションの設定を定義する
 */
USTRUCT(BlueprintType)
struct GCLADDON_API FAnimRotateInPlaceConfigs
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ClampMax = 180, ForceUnits = "deg"))
	float ViewYawAngleThreshold = 50.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0))
	FVector2f ReferenceViewYawSpeed = { 180.0, 460.0 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0))
	FVector2f PlayRate { 1.15, 3.0 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDisableFootLock = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ClampMax = 180, EditCondition = "!bDisableFootLock", ForceUnits = "deg"))
	float FootLockBlockViewYawAngleThreshold = 120.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, EditCondition = "!bDisableFootLock", ForceUnits = "deg/s"))
	float FootLockBlockViewYawSpeedThreshold = 620.0;
};


/**
 * FAnimTransitionsConfigs
 *
 *  遷移アニメーションの設定を定義する
 */
USTRUCT(BlueprintType)
struct GCLADDON_API FAnimTransitionsConfigs
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "s"))
	float QuickStopBlendInDuration = 0.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "s"))
	float QuickStopBlendOutDuration = 0.2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0))
	FVector2f QuickStopPlayRate = { 1.75f, 3.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "s"))
	float QuickStopStartTime = 0.3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequenceBase> StandingTransitionLeftAnimation{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequenceBase> StandingTransitionRightAnimation{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequenceBase> CrouchingTransitionLeftAnimation{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequenceBase> CrouchingTransitionRightAnimation{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "cm"))
	float DynamicTransitionFootLockDistanceThreshold = 8.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "s"))
	float DynamicTransitionBlendDuration = 0.2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ForceUnits = "x"))
	float DynamicTransitionPlayRate = 1.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequenceBase> StandingDynamicTransitionLeftAnimation{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequenceBase> StandingDynamicTransitionRightAnimation{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequenceBase> CrouchingDynamicTransitionLeftAnimation{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequenceBase> CrouchingDynamicTransitionRightAnimation{ nullptr };
};


/**
 * UCharacterAnimData
 *
 *  キャラクターのアニメーションの設定を定義する
 */
UCLASS(Blueprintable, BlueprintType)
class GCLADDON_API UCharacterAnimData : public UDataAsset
{
	GENERATED_BODY()
public:
	UCharacterAnimData();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FCharacterAnimGeneralConfigs General;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FCharacterAnimViewConfigs View;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FCharacterAnimGroundedConfigs Grounded;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAnimInAirConfigs InAir;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FCharacterAnimFeetConfigs Feet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAnimTransitionsConfigs Transitions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAnimRotateInPlaceConfigs RotateInPlace;
};

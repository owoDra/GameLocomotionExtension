// Copyright (C) 2023 owoDra

#include "LocomotionFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LocomotionFunctionLibrary)


float ULocomotionFunctionLibrary::Clamp01(float Value)
{
	return Value <= 0.0f
		? 0.0f
		: Value >= 1.0f
		? 1.0f
		: Value;
}

float ULocomotionFunctionLibrary::LerpClamped(float From, float To, float Alpha)
{
	return From + (To - From) * Clamp01(Alpha);
}

float ULocomotionFunctionLibrary::LerpAngle(float From, float To, float Alpha)
{
	auto Delta{ FRotator3f::NormalizeAxis(To - From) };

	if (Delta > 180.0f - CounterClockwiseRotationAngleThreshold)
	{
		Delta -= 360.0f;
	}

	return FRotator3f::NormalizeAxis(From + Delta * Alpha);
}

FRotator ULocomotionFunctionLibrary::LerpRotator(const FRotator& From, const FRotator& To, float Alpha)
{
	auto Result{ To - From };
	Result.Normalize();

	if (Result.Pitch > 180.0f - CounterClockwiseRotationAngleThreshold)
	{
		Result.Pitch -= 360.0f;
	}

	if (Result.Yaw > 180.0f - CounterClockwiseRotationAngleThreshold)
	{
		Result.Yaw -= 360.0f;
	}

	if (Result.Roll > 180.0f - CounterClockwiseRotationAngleThreshold)
	{
		Result.Roll -= 360.0f;
	}

	Result *= Alpha;
	Result += From;
	Result.Normalize();

	return Result;
}

float ULocomotionFunctionLibrary::ExponentialDecay(float DeltaTime, float Lambda)
{
	return 1.0f - FMath::InvExpApprox(Lambda * DeltaTime);
}

float ULocomotionFunctionLibrary::ExponentialDecayAngle(float Current, float Target, float DeltaTime, float Lambda)
{
	return Lambda > 0.0f
		? LerpAngle(Current, Target, ExponentialDecay(DeltaTime, Lambda))
		: Target;
}

float ULocomotionFunctionLibrary::InterpolateAngleConstant(float Current, float Target, float DeltaTime, float InterpolationSpeed)
{
	if (InterpolationSpeed <= 0.0f || Current == Target)
	{
		return Target;
	}

	auto Delta{ FRotator3f::NormalizeAxis(Target - Current) };

	if (Delta > 180.0f - CounterClockwiseRotationAngleThreshold)
	{
		Delta -= 360.0f;
	}

	const auto Alpha{ InterpolationSpeed * DeltaTime };

	return FRotator3f::NormalizeAxis(Current + FMath::Clamp(Delta, -Alpha, Alpha));
}

FVector ULocomotionFunctionLibrary::ClampMagnitude01(const FVector& Vector)
{
	const auto MagnitudeSquared{ Vector.SizeSquared() };

	if (MagnitudeSquared <= 1.0f)
	{
		return Vector;
	}

	const auto Scale{ FMath::InvSqrt(MagnitudeSquared) };

	return FVector(Vector.X * Scale, Vector.Y * Scale, Vector.Z * Scale);
}

FVector3f ULocomotionFunctionLibrary::ClampMagnitude01(const FVector3f& Vector)
{
	const auto MagnitudeSquared{ Vector.SizeSquared() };

	if (MagnitudeSquared <= 1.0f)
	{
		return Vector;
	}

	const auto Scale{ FMath::InvSqrt(MagnitudeSquared) };

	return FVector3f(Vector.X * Scale, Vector.Y * Scale, Vector.Z * Scale);
}

FVector ULocomotionFunctionLibrary::RadianToDirectionXY(float Radian)
{
	float Sin, Cos;
	FMath::SinCos(&Sin, &Cos, Radian);

	return FVector(Cos, Sin, 0.0f);
}

FVector ULocomotionFunctionLibrary::AngleToDirectionXY(float Angle)
{
	return RadianToDirectionXY(FMath::DegreesToRadians(Angle));
}

double ULocomotionFunctionLibrary::DirectionToAngle(const FVector2D& Direction)
{
	return FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
}

double ULocomotionFunctionLibrary::DirectionToAngleXY(const FVector& Direction)
{
	return FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
}

FVector ULocomotionFunctionLibrary::PerpendicularClockwiseXY(const FVector& Vector)
{
	return { Vector.Y, -Vector.X, Vector.Z };
}

FVector ULocomotionFunctionLibrary::PerpendicularCounterClockwiseXY(const FVector& Vector)
{
	return { -Vector.Y, Vector.X, Vector.Z };
}

double ULocomotionFunctionLibrary::AngleBetweenSkipNormalization(const FVector& From, const FVector& To)
{
	return FMath::RadiansToDegrees(FMath::Acos(From | To));
}

FVector ULocomotionFunctionLibrary::SlerpSkipNormalization(const FVector& From, const FVector& To, float Alpha)
{
	const auto Dot{ From | To };

	if (Dot > 0.9995f || Dot < -0.9995f)
	{
		return FMath::Lerp(From, To, Alpha).GetSafeNormal();
	}

	const auto Theta{ UE_REAL_TO_FLOAT(FMath::Acos(Dot)) * Alpha };

	float Sin, Cos;
	FMath::SinCos(&Sin, &Cos, Theta);

	const auto FromPerpendicular{ (To - From * Dot).GetSafeNormal() };

	return From * Cos + FromPerpendicular * Sin;
}

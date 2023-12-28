#include "MovementMathLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MovementMathLibrary)


float UMovementMathLibrary::Clamp01(float Value)
{
	return Value <= 0.0f
		? 0.0f
		: Value >= 1.0f
		? 1.0f
		: Value;
}

float UMovementMathLibrary::LerpClamped(float From, float To, float Alpha)
{
	return From + (To - From) * Clamp01(Alpha);
}

float UMovementMathLibrary::LerpAngle(float From, float To, float Alpha)
{
	auto Delta{ FRotator3f::NormalizeAxis(To - From) };

	if (Delta > 180.0f - CounterClockwiseRotationAngleThreshold)
	{
		Delta -= 360.0f;
	}

	return FRotator3f::NormalizeAxis(From + Delta * Alpha);
}

FRotator UMovementMathLibrary::LerpRotator(const FRotator& From, const FRotator& To, float Alpha)
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

float UMovementMathLibrary::ExponentialDecay(float DeltaTime, float Lambda)
{
	return 1.0f - FMath::InvExpApprox(Lambda * DeltaTime);
}

float UMovementMathLibrary::ExponentialDecayAngle(float Current, float Target, float DeltaTime, float Lambda)
{
	return Lambda > 0.0f
		? LerpAngle(Current, Target, ExponentialDecay(DeltaTime, Lambda))
		: Target;
}

float UMovementMathLibrary::InterpolateAngleConstant(float Current, float Target, float DeltaTime, float InterpolationSpeed)
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

float UMovementMathLibrary::SpringDampFloat(float Current, float Target, UPARAM(ref)FSpringFloatState& SpringState, float DeltaTime, float Frequency, float DampingRatio, float TargetVelocityAmount)
{
	return SpringDamp(Current, Target, SpringState, DeltaTime, Frequency, DampingRatio, TargetVelocityAmount);
}

FVector UMovementMathLibrary::SpringDampVector(const FVector& Current, const FVector& Target, UPARAM(ref)FSpringVectorState& SpringState, float DeltaTime, float Frequency, float DampingRatio, float TargetVelocityAmount)
{
	return SpringDamp(Current, Target, SpringState, DeltaTime, Frequency, DampingRatio, TargetVelocityAmount);
}

FVector UMovementMathLibrary::ClampMagnitude01(const FVector& Vector)
{
	const auto MagnitudeSquared{ Vector.SizeSquared() };

	if (MagnitudeSquared <= 1.0f)
	{
		return Vector;
	}

	const auto Scale{ FMath::InvSqrt(MagnitudeSquared) };

	return FVector(Vector.X * Scale, Vector.Y * Scale, Vector.Z * Scale);
}

FVector3f UMovementMathLibrary::ClampMagnitude01(const FVector3f& Vector)
{
	const auto MagnitudeSquared{ Vector.SizeSquared() };

	if (MagnitudeSquared <= 1.0f)
	{
		return Vector;
	}

	const auto Scale{ FMath::InvSqrt(MagnitudeSquared) };

	return FVector3f(Vector.X * Scale, Vector.Y * Scale, Vector.Z * Scale);
}

FVector UMovementMathLibrary::RadianToDirectionXY(float Radian)
{
	float Sin, Cos;
	FMath::SinCos(&Sin, &Cos, Radian);

	return FVector(Cos, Sin, 0.0f);
}

FVector UMovementMathLibrary::AngleToDirectionXY(float Angle)
{
	return RadianToDirectionXY(FMath::DegreesToRadians(Angle));
}

double UMovementMathLibrary::DirectionToAngle(const FVector2D& Direction)
{
	return FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
}

double UMovementMathLibrary::DirectionToAngleXY(const FVector& Direction)
{
	return FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
}

FVector UMovementMathLibrary::PerpendicularClockwiseXY(const FVector& Vector)
{
	return { Vector.Y, -Vector.X, Vector.Z };
}

FVector UMovementMathLibrary::PerpendicularCounterClockwiseXY(const FVector& Vector)
{
	return { -Vector.Y, Vector.X, Vector.Z };
}

double UMovementMathLibrary::AngleBetweenSkipNormalization(const FVector& From, const FVector& To)
{
	return FMath::RadiansToDegrees(FMath::Acos(From | To));
}

FVector UMovementMathLibrary::SlerpSkipNormalization(const FVector& From, const FVector& To, float Alpha)
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

EMovementDirection UMovementMathLibrary::CalculateMovementDirection(float Angle, float ForwardHalfAngle, float AngleThreshold)
{
	if (Angle >= -ForwardHalfAngle - AngleThreshold && Angle <= ForwardHalfAngle + AngleThreshold)
	{
		return EMovementDirection::Forward;
	}

	if (Angle >= ForwardHalfAngle - AngleThreshold && Angle <= 180.0f - ForwardHalfAngle + AngleThreshold)
	{
		return EMovementDirection::Right;
	}

	if (Angle <= -(ForwardHalfAngle - AngleThreshold) && Angle >= -(180.0f - ForwardHalfAngle + AngleThreshold))
	{
		return EMovementDirection::Left;
	}

	return EMovementDirection::Backward;
}

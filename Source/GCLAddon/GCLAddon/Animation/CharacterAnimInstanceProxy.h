#pragma once

#include "Animation/AnimInstanceProxy.h"

#include "CharacterAnimInstanceProxy.generated.h"

class UCharacterAnimInstance;
class UCharacterLinkedAnimInstance;

/**
 * This class is used to access some protected members of the FAnimInstanceProxy when using UCharacterAnimInstance and UCharacterLinkedAnimInstance
 */
USTRUCT()
struct GCLADDON_API FCharacterAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

	friend UCharacterAnimInstance;
	friend UCharacterLinkedAnimInstance;

public:
	FCharacterAnimInstanceProxy() = default;

	explicit FCharacterAnimInstanceProxy(UAnimInstance* AnimationInstance);

};

#include "CharacterLinkedAnimInstance.h"

#include "CharacterAnimInstance.h"
#include "CharacterAnimInstanceProxy.h"

#include "CharacterMeshAccessorInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterLinkedAnimInstance)


UCharacterLinkedAnimInstance::UCharacterLinkedAnimInstance()
{
	RootMotionMode = ERootMotionMode::IgnoreRootMotion;
	bUseMainInstanceMontageEvaluationData = true;
}


void UCharacterLinkedAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<AGCLACharacter>(GetOwningActor());
	
#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld())
	{
		// Use default object for editor preview.

		if (!Parent.IsValid())
		{
			Parent = GetMutableDefault<UCharacterAnimInstance>();
		}

		if (!IsValid(Character))
		{
			Character = GetMutableDefault<AGCLACharacter>();
		}
		return;
	}
#endif

	auto* Mesh{ ICharacterMeshAccessorInterface::Execute_GetMainMesh(this) };
	
	Parent = Mesh ? Cast<UCharacterAnimInstance>(Mesh->GetAnimInstance()) : nullptr;
}

void UCharacterLinkedAnimInstance::NativeBeginPlay()
{
	ensureMsgf(Parent.IsValid(), TEXT("Parent is invalid. Parent must inherit from UCharacterAnimInstance."));

	Super::NativeBeginPlay();
}

FAnimInstanceProxy* UCharacterLinkedAnimInstance::CreateAnimInstanceProxy()
{
	return new FCharacterAnimInstanceProxy(this);
}

UCharacterAnimInstance* UCharacterLinkedAnimInstance::GetParentUnsafe() const
{
	return Parent.Get();
}

void UCharacterLinkedAnimInstance::ReinitializeLook()
{
	if (Parent.IsValid())
	{
		Parent->ReinitializeLook();
	}
}

void UCharacterLinkedAnimInstance::RefreshLook()
{
	if (Parent.IsValid())
	{
		Parent->UpdateLook();
	}
}

void UCharacterLinkedAnimInstance::SetHipsDirection(const EHipsDirection NewHipsDirection)
{
	if (Parent.IsValid())
	{
		Parent->SetHipsDirection(NewHipsDirection);
	}
}

void UCharacterLinkedAnimInstance::ActivatePivot()
{
	if (Parent.IsValid())
	{
		Parent->ActivatePivot();
	}
}

void UCharacterLinkedAnimInstance::ResetJumped()
{
	if (Parent.IsValid())
	{
		Parent->ResetJumped();
	}
}

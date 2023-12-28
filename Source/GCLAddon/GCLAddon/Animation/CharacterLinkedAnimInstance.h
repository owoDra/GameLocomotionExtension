#pragma once

#include "Animation/AnimInstance.h"

#include "CharacterLinkedAnimInstance.generated.h"

class AGCLACharacter;
class UCharacterAnimInstance;
enum class EHipsDirection : uint8;

/**
 * Sub animations without main state processing for Linked Anim Layers applied to Character
 * 
 * Tips:
 *  Can be used as AnimInstance for FPP in addition to Linked Anim Layers
 */
UCLASS()
class GCLADDON_API UCharacterLinkedAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UCharacterLinkedAnimInstance();

protected:
	//
	// AnimInstance that handles the main processing of the Character.
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Refarence", Transient)
	TWeakObjectPtr<UCharacterAnimInstance> Parent;

	//
	// The Character that owns this AnimInstance.
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Refarence", Transient)
	TObjectPtr<AGCLACharacter> Character;

public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeBeginPlay() override;

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;

protected:
	//
	// Use PropertyAccess to read the main AnimInstace variable
	// 
	// ========== WARNNING =========
	// Since this function is guaranteed to be called before parallel animation evaluation
	// It is safe to read variables that are modified only inside Parent's UCharacterAnimationInstance::NativeUpdateAnimation()
	// 
	// If you don't know what you are doing, access the variable through the Parent variable
	//
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = " Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe, ReturnDisplayName = "Parent"))
	UCharacterAnimInstance* GetParentUnsafe() const;

	UFUNCTION(BlueprintCallable, Category = " Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ReinitializeLook();

	UFUNCTION(BlueprintCallable, Category = " Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void RefreshLook();

	UFUNCTION(BlueprintCallable, Category = " Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void SetHipsDirection(EHipsDirection NewHipsDirection);

	UFUNCTION(BlueprintCallable, Category = " Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ActivatePivot();

	UFUNCTION(BlueprintCallable, Category = " Linked Animation Instance", Meta = (BlueprintProtected, BlueprintThreadSafe))
	void ResetJumped();

};

// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/WGasBlock.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Animation/AnimInstance.h"
#include "Animation/WGasAnimLayerInterface.h"
#include "Character/WGasCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WGasGameplayTags.h"
#include "AbilitySystem/Abilities/WGasMeleeAttack.h"

UWGasBlock::UWGasBlock()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
	if (Tags.State_Block.IsValid())
	{
		ActivationBlockedTags.AddTag(Tags.State_Block);
	}
	if (Tags.State_Dodge.IsValid())
	{
		ActivationBlockedTags.AddTag(Tags.State_Dodge);
	}
	if (Tags.State_Attacking_Active.IsValid())
	{
		ActivationBlockedTags.AddTag(Tags.State_Attacking_Active);
	}
}

void UWGasBlock::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		if (UWGasMeleeAttack* Melee = UWGasMeleeAttack::GetActiveMeleeAttack(ASC))
		{
			Melee->TryCancelFromCancelablePhase();
		}
	}
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	bBlockEndHandled = false;
	bParryTriggered = false;

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		if (FWGasGameplayTags::Get().State_Block.IsValid())
		{
			ASC->AddLooseGameplayTag(FWGasGameplayTags::Get().State_Block);
		}

		if (BlockStaminaDrainEffectClass)
		{
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddSourceObject(GetWGasCharacterFromActorInfo());
			const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(BlockStaminaDrainEffectClass, 1.f, ContextHandle);
			if (Spec.IsValid())
			{
				BlockStaminaDrainEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}

	if (AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo())
	{
		ApplyBlockMovementPenalty(Character);
	}
}

UWGasBlock* UWGasBlock::GetActiveBlock(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return nullptr;
	}

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (!Spec.IsActive())
		{
			continue;
		}

		for (UGameplayAbility* Instance : Spec.GetAbilityInstances())
		{
			if (UWGasBlock* Block = Cast<UWGasBlock>(Instance))
			{
				return Block;
			}
		}

		if (UGameplayAbility* Instance = Spec.GetPrimaryInstance())
		{
			if (UWGasBlock* Block = Cast<UWGasBlock>(Instance))
			{
				return Block;
			}
		}
	}

	return nullptr;
}

void UWGasBlock::TryParryFromAttackInput()
{
	if (bBlockEndHandled || bParryTriggered)
	{
		return;
	}
	bParryTriggered = true;
	//弹刀加上parry状态
	if (UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo())
	{
		const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
		if (Tags.State_Parry.IsValid())
		{
			ASC->AddLooseGameplayTag(Tags.State_Parry);
		}
	}
	AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo();
	SetParryAnimationStateOnCharacter(Character, true);
}

float UWGasBlock::GetBlockDamageMultiplierForTarget(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return 1.f;
	}
	const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
	if (!Tags.State_Block.IsValid() || !ASC->HasMatchingGameplayTag(Tags.State_Block))
	{
		return 1.f;
	}
	if (const UWGasBlock* Block = GetActiveBlock(ASC))
	{
		return Block->BlockDamageMultiplier;
	}
	return 0.3f; // 有 Tag 但拿不到 GA 实例时的默认值
}

void UWGasBlock::EndBlockActivePhase()
{
	FinishBlock();
}

void UWGasBlock::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled)
{
	if (!bBlockEndHandled)
	{
		bBlockEndHandled = true;

		AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo();
		RestoreBlockMovementPenalty(Character);
		RemoveBlockStaminaDrainEffect();
		SetBlockAnimationStateOnCharacter(Character, false);
		SetParryAnimationStateOnCharacter(Character, false);

		if (UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo())
		{
			const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
			if (Tags.State_Block.IsValid())
			{
				ASC->RemoveLooseGameplayTag(Tags.State_Block);
			}
			if (Tags.Player_Block_InputPressed.IsValid())
			{
				ASC->RemoveLooseGameplayTag(Tags.Player_Block_InputPressed);
			}
			if (Tags.Player_Block_InputHeld.IsValid())
			{
				ASC->RemoveLooseGameplayTag(Tags.Player_Block_InputHeld);
			}
			if (Tags.Player_Block_InputReleased.IsValid())
			{
				ASC->RemoveLooseGameplayTag(Tags.Player_Block_InputReleased);
			}

			if (Tags.State_Parry.IsValid())
			{
				ASC->RemoveLooseGameplayTag(Tags.State_Parry);
			}
			if (Tags.State_Parry_Window.IsValid())
			{
				ASC->RemoveLooseGameplayTag(Tags.State_Parry_Window);
			}
			if (Tags.State_Parry_Success.IsValid())
			{
				ASC->RemoveLooseGameplayTag(Tags.State_Parry_Success);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UWGasBlock::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	if (bParryTriggered)
	{
		// 弹反中松 RMB：只通知 GAS 输入已松开，等 AnimBP EndBlockFromAnimation 再 EndAbility。
		return;
	}

	FinishBlock();
}

void UWGasBlock::SetBlockAnimationStateOnCharacter(AWGasCharacterBase* Character, const bool bIsBlocking) const
{
	if (!Character)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh)
	{
		return;
	}

	UAnimInstance* Anim = Mesh->GetAnimInstance();
	if (!Anim || !Anim->GetClass()->ImplementsInterface(UWGasAnimLayerInterface::StaticClass()))
	{
		return;
	}

	IWGasAnimLayerInterface::Execute_SetBlockAnimationState(Anim, bIsBlocking);
}

void UWGasBlock::SetParryAnimationStateOnCharacter(AWGasCharacterBase* Character, const bool bIsParrying) const
{
	if (!Character)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh)
	{
		return;
	}

	UAnimInstance* Anim = Mesh->GetAnimInstance();
	if (!Anim || !Anim->GetClass()->ImplementsInterface(UWGasAnimLayerInterface::StaticClass()))
	{
		return;
	}

	IWGasAnimLayerInterface::Execute_SetParryAnimationState(Anim, bIsParrying);
}

void UWGasBlock::ApplyBlockMovementPenalty(AWGasCharacterBase* Character)
{
	if (!Character || bBlockMovementPenaltyApplied)
	{
		return;
	}

	if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
	{
		CachedMaxWalkSpeed = Movement->MaxWalkSpeed;
		Movement->MaxWalkSpeed = CachedMaxWalkSpeed * BlockMoveSpeedMultiplier;
		bBlockMovementPenaltyApplied = true;
	}

	SetBlockAnimationStateOnCharacter(Character, true);
}

void UWGasBlock::RestoreBlockMovementPenalty(AWGasCharacterBase* Character)
{
	if (!Character || !bBlockMovementPenaltyApplied)
	{
		return;
	}

	if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = CachedMaxWalkSpeed;
	}

	bBlockMovementPenaltyApplied = false;
	CachedMaxWalkSpeed = 0.f;
}

void UWGasBlock::RemoveBlockStaminaDrainEffect()
{
	if (!BlockStaminaDrainEffectHandle.IsValid())
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo())
	{
		ASC->RemoveActiveGameplayEffect(BlockStaminaDrainEffectHandle);
	}

	BlockStaminaDrainEffectHandle = FActiveGameplayEffectHandle();
}

void UWGasBlock::FinishBlock()
{
	if (bBlockEndHandled)
	{
		return;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

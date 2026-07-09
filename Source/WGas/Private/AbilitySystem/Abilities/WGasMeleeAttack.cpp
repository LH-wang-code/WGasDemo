// Fill out your copyright notice in the Description page of Project Settings.



#include "AbilitySystem/Abilities/WGasMeleeAttack.h"



#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include "Animation/AnimInstance.h"

#include "Animation/AnimMontage.h"

#include "Character/WGasCharacterBase.h"

#include "Components/SkeletalMeshComponent.h"

#include "DrawDebugHelpers.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "KismetTraceUtils.h"
#include "WGas.h"
#include "WGasGameplayTags.h"
#include "Character/WGasCharacterHero.h"
#include "Character/WGasLockOnComponent.h"
#include "Input/WGasInputComponent.h"


UWGasMeleeAttack::UWGasMeleeAttack()

{

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

}

void UWGasMeleeAttack::InputPressed(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	if (AWGasCharacterHero* Character=Cast<AWGasCharacterHero>(GetWGasCharacterFromActorInfo()))
	{
		if (UGasInputBufferComponent*Buffer=Character->GetWGasInputBufferComponent())
		{
			//只有左键激活这个能力，检测到输入时我们直接放进去tag就行
			Buffer->BufferInput(FWGasGameplayTags::Get().InputTag_LMB);
		}
	}
	OnComboInputReceived();
}

void UWGasMeleeAttack::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
}


void UWGasMeleeAttack::BeginMeleeAttack()

{
	if (AWGasCharacterHero* Hero=Cast<AWGasCharacterHero>(GetWGasCharacterFromActorInfo()))
	{
		if (UWGasLockOnComponent* LockComp=Hero->FindComponentByClass<UWGasLockOnComponent>())
		{
			if (LockComp->IsLockedOn())
			{
				if (AActor* Target = LockComp->GetLockTarget())
				{
					const FVector ToTarget = LockComp->GetCurrentLockOnLocation() - Hero->GetActorLocation();
					if (!ToTarget.IsNearlyZero())
					{
						Hero->SetActorRotation(FRotator(0.f, ToTarget.Rotation().Yaw, 0.f));

					}
				}
			}
		}
	}
	ApplyAttackingTags();
	if (!bStopMovementOnAttack)
	{
		return;
	}
	if (AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo())
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}

		RegisterDamageWindow();
		
	}
}

void UWGasMeleeAttack::EndMeleeAttack(bool bWasCancelled)
{
	RemoveAttackingTags();
	if (bWasCancelled)
	{
		K2_CancelAbility();
	}
	else
	{
		K2_EndAbility();
	}
	UnregisterDamageWindow();
	
}
void UWGasMeleeAttack::OnMeleeMontageFinished(bool bWasCancelled)
{
	EndMeleeAttack(bWasCancelled);
}
void UWGasMeleeAttack::ApplyAttackingTags() const
{
	if (!AttackingStateTag.IsValid())
	{
		return;
	}
	if (UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo())
	{
		ASC->AddLooseGameplayTag(AttackingStateTag);
	}
}
void UWGasMeleeAttack::RemoveAttackingTags() const
{
	if (!AttackingStateTag.IsValid())
	{
		return;
	}
	if (UAbilitySystemComponent* ASC = GetWGasASCFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(AttackingStateTag);
	}
}



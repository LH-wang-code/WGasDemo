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
	}
}
void UWGasMeleeAttack::PerformMeleeHitCheck()
{
	AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo();

	if (!Character)
	{
		return;
	}
	const FVector TraceStart = Character->GetActorLocation();
	const FVector TraceEnd = TraceStart + Character->GetActorForwardVector() * MeleeTraceDistance;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MeleeAttackTrace), false, Character);
	FCollisionShape Sphere = FCollisionShape::MakeSphere(MeleeTraceRadius);
	FHitResult HitResult;
	const bool bHit = Character->GetWorld()->SweepSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		ECC_Pawn,
		Sphere,
		QueryParams);



#if ENABLE_DRAW_DEBUG

	DrawDebugSweptSphere(

		Character->GetWorld(),

		TraceStart,

		TraceEnd,

		MeleeTraceRadius,

		FColor::Red,

		false,

		1.f);

#endif
	if (!bHit)
	{
		return;
	}
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor || HitActor == Character)
	{
		return;
	}
	
	// 后续在这里应用 GameplayEffect 伤害
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



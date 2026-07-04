// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/WGasDodge.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/WGasCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WGasGameplayTags.h"

UWGasDodge::UWGasDodge()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FWGasGameplayTags& Tags = FWGasGameplayTags::Get();
	if (Tags.State_Dodge.IsValid())
	{
		ActivationBlockedTags.AddTag(Tags.State_Dodge);
	}
	if (Tags.State_Attacking_Lighting.IsValid())
	{
		ActivationBlockedTags.AddTag(Tags.State_Attacking_Lighting);
	}
}

FVector UWGasDodge::GetDodgeDirection(const AWGasCharacterBase* Character) const
{
	if (!Character)
	{
		return FVector::ForwardVector;
	}

	FVector DodgeDir = FVector::ZeroVector;

	if (const UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
	{
		// 优先用当前帧待处理的移动输入（与 WASD 一致）
		DodgeDir = MoveComp->GetPendingInputVector();
		if (DodgeDir.IsNearlyZero())
		{
			DodgeDir = MoveComp->GetLastInputVector();
		}
	}

	if (DodgeDir.IsNearlyZero())
	{
		DodgeDir = Character->GetLastMovementInputVector();
	}

	DodgeDir.Z = 0.f;

	if (!DodgeDir.Normalize())
	{
		DodgeDir = -Character->GetActorForwardVector();
		DodgeDir.Z = 0.f;
		DodgeDir.Normalize();
	}

	return DodgeDir;
}

UAnimMontage* UWGasDodge::GetDodgeMontageForDirection(const FVector& DodgeDir,
	const AWGasCharacterBase* Character) const
{
	auto ResolveMontage = [](UAnimMontage* Preferred, UAnimMontage* FallbackA, UAnimMontage* FallbackB)
	{
		if (Preferred)
		{
			return Preferred;
		}
		return FallbackA ? FallbackA : FallbackB;
	};

	if (!Character)
	{
		return DodgeBackMontage;
	}

	const float CharYaw = Character->GetActorRotation().Yaw;
	const float DodgeYaw = DodgeDir.Rotation().Yaw;
	const float Delta = FMath::FindDeltaAngleDegrees(CharYaw, DodgeYaw);

	// 8 方向，每扇区 45°（相对角色朝向）
	static constexpr float SectorHalfAngle = 22.5f;

	if (Delta > -SectorHalfAngle && Delta <= SectorHalfAngle)
	{
		return DodgeForwardMontage;
	}
	if (Delta > SectorHalfAngle && Delta <= 45.f + SectorHalfAngle)
	{
		return ResolveMontage(DodgeForwardRightMontage, DodgeForwardMontage, DodgeRightMontage);
	}
	if (Delta > 45.f + SectorHalfAngle && Delta <= 90.f + SectorHalfAngle)
	{
		return DodgeRightMontage;
	}
	if (Delta > 90.f + SectorHalfAngle && Delta <= 135.f + SectorHalfAngle)
	{
		return ResolveMontage(DodgeBackRightMontage, DodgeBackMontage, DodgeRightMontage);
	}
	if (Delta > -45.f - SectorHalfAngle && Delta <= -SectorHalfAngle)
	{
		return ResolveMontage(DodgeForwardLeftMontage, DodgeForwardMontage, DodgeLeftMontage);
	}
	if (Delta > -90.f - SectorHalfAngle && Delta <= -45.f - SectorHalfAngle)
	{
		return DodgeLeftMontage;
	}
	if (Delta > -135.f - SectorHalfAngle && Delta <= -90.f - SectorHalfAngle)
	{
		return ResolveMontage(DodgeBackLeftMontage, DodgeBackMontage, DodgeLeftMontage);
	}
	return DodgeBackMontage;
}

void UWGasDodge::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo();
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	bDodgeEndHandled = false;

	// 先取方向再 StopMovement，避免清掉输入后方向变成默认后撤
	const FVector DodgeDir = GetDodgeDirection(Character);

	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		if (FWGasGameplayTags::Get().State_Dodge.IsValid())
		{
			ASC->AddLooseGameplayTag(FWGasGameplayTags::Get().State_Dodge);
		}
	}

	if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}

	if (bUseLaunchImpulse)
	{
		PerformDodge();
	}

	UAnimMontage* MontageToPlay = GetDodgeMontageForDirection(DodgeDir, Character);
	if (!MontageToPlay)
	{
		OnDodgeMontageEnded();
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("DodgeMontageTask"),
			MontageToPlay,
			1.0f,
			NAME_None);
	MontageTask->OnCompleted.AddDynamic(this, &UWGasDodge::OnDodgeMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UWGasDodge::OnDodgeMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UWGasDodge::OnDodgeMontageEnded);
	MontageTask->ReadyForActivation();
}

void UWGasDodge::OnDodgeMontageEnded()
{
	if (bDodgeEndHandled)
	{
		return;
	}
	bDodgeEndHandled = true;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (FWGasGameplayTags::Get().State_Dodge.IsValid())
		{
			ASC->RemoveLooseGameplayTag(FWGasGameplayTags::Get().State_Dodge);
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UWGasDodge::PerformDodge()
{
	AWGasCharacterBase* Character = GetWGasCharacterFromActorInfo();
	if (!Character)
	{
		return;
	}

	const FVector DodgeDir = GetDodgeDirection(Character);
	Character->LaunchCharacter(DodgeDir * DodgeLaunchPower, true, true);
}
